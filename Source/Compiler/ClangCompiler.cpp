#include "ClangCompiler.h"

#include <filesystem>
#include <cstdlib>
#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <unordered_set>
#include <vector>

#include "Compiler/Compilation.h"
#include "Compiler/CompileCommandsExporter.h"
#include "Core/Logger.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

#ifdef _WIN32
static std::string QuoteIfNeeded(const std::string& s)
{
    if (s.find(' ') != std::string::npos)
        return "\"" + s + "\"";
    return s;
}
#endif

#ifdef _WIN32
    static fs::path clangPath = "clang.exe";
    static fs::path clangCppPath = "clang++.exe";
    static fs::path arPath = "llvm-ar.exe";
    static std::string objExt = ".obj";
    static std::string libExt = ".lib";
#else
    static fs::path clangPath = "clang";
    static fs::path clangCppPath = "clang++";
    static fs::path arPath = "ar";
    static std::string objExt = ".o";
    static std::string libExt = ".a";
#endif

enum FileLanguage : unsigned int
{
    LANG_C,
    LANG_CPP
};

namespace
{
    std::string MakeFileId(const fs::path& file)
    {
        return file.filename();
        //try
        //{
        //    return fs::weakly_canonical(file).lexically_normal().string();
        //}
        //catch (const fs::filesystem_error&)
        //{
        //    return fs::absolute(file).lexically_normal().string();
        //}
    }

    fs::path ResolveIncludePath(const std::string& includeName, const fs::path& sourceDir,
                                const std::vector<fs::path>& includePaths)
    {
        std::vector<fs::path> searchRoots;
        searchRoots.reserve(includePaths.size() + 1);
        searchRoots.push_back(sourceDir);
        searchRoots.insert(searchRoots.end(), includePaths.begin(), includePaths.end());

        for (const auto& root : searchRoots)
        {
            fs::path candidate = root / includeName;
            if (!fs::exists(candidate) || !fs::is_regular_file(candidate))
                continue;

            try
            {
                return fs::weakly_canonical(candidate);
            }
            catch (const fs::filesystem_error&)
            {
                return fs::absolute(candidate);
            }
        }

        return {};
    }

    void CollectFileDependenciesRecursive(const fs::path& file,
                                          const std::vector<fs::path>& includePaths,
                                          std::unordered_set<std::string>& visitedFiles,
                                          std::vector<fs::path>& dependencies)
    {
        std::ifstream input(file);
        if (!input)
            return;

        static const std::regex includePattern(R"(^\s*#\s*include\s*[<"]([^">]+)[">])");
        std::string line;
        while (std::getline(input, line))
        {
            std::smatch match;
            if (!std::regex_search(line, match, includePattern))
                continue;

            fs::path dependency = ResolveIncludePath(match[1].str(), file.parent_path(), includePaths);
            if (dependency.empty())
                continue;

            const std::string dependencyId = MakeFileId(dependency);
            if (!visitedFiles.insert(dependencyId).second)
                continue;

            dependencies.push_back(dependency);
            CollectFileDependenciesRecursive(dependency, includePaths, visitedFiles, dependencies);
        }
    }

    std::vector<fs::path> CollectFileDependencies(const fs::path& sourceFile,
                                                  const std::vector<fs::path>& includePaths)
    {
        std::unordered_set<std::string> visitedFiles;
        visitedFiles.insert(MakeFileId(sourceFile));

        std::vector<fs::path> dependencies;
        CollectFileDependenciesRecursive(sourceFile, includePaths, visitedFiles, dependencies);
        return dependencies;
    }

    std::vector<std::string> BuildDependencyIds(const std::vector<fs::path>& dependencies)
    {
        std::vector<std::string> dependencyIds;
        dependencyIds.reserve(dependencies.size());

        for (const auto& dependency : dependencies)
            dependencyIds.push_back(MakeFileId(dependency));

        return dependencyIds;
    }
}

static void ExecuteCommand(const fs::path& exePath, const std::vector<std::string>& argStrings)
{
    std::stringstream ss;
    ss << "Starting command : ";
    for (const auto& arg : argStrings)
        ss << " " << arg;
    Logger::Log(LogLevel::Debug, "%s", ss.str().c_str());

#ifdef _WIN32
    std::string command;
    command += QuoteIfNeeded(exePath.string());

    for (size_t i = 1; i < argStrings.size(); ++i)
    {
        command += " ";
        command += QuoteIfNeeded(argStrings[i]);
    }

    int ret = std::system(command.c_str());
    if (ret != 0)
    {
        Logger::Log(LogLevel::Error, "Command failed with code %d", ret);
        throw std::runtime_error("Command failed with code " + std::to_string(ret) + ": " + exePath.string());
    }
#else
    std::vector<char*> args;
    for (auto& s : argStrings)
        args.push_back(const_cast<char*>(s.c_str()));
    args.push_back(nullptr);

    pid_t pid = fork();
    if (pid < 0)
    {
        throw std::runtime_error("Unable to fork process for command: " + exePath.string());
    }

    if (pid == 0)
    {
        execvp(exePath.c_str(), args.data());
        perror("execvp");
        exit(1);
    }
    else
    {
        int status = 0;
        if (waitpid(pid, &status, 0) < 0)
        {
            throw std::runtime_error("Unable to wait for command: " + exePath.string());
        }

        if (WIFEXITED(status))
        {
            const int exitCode = WEXITSTATUS(status);
            if (exitCode != 0)
            {
                Logger::Log(LogLevel::Error, "Command failed with code %d", exitCode);
                throw std::runtime_error("Command failed with code " + std::to_string(exitCode) + ": " + exePath.string());
            }
        }
        else if (WIFSIGNALED(status))
        {
            const int signalCode = WTERMSIG(status);
            Logger::Log(LogLevel::Error, "Command terminated by signal %d", signalCode);
            throw std::runtime_error("Command terminated by signal " + std::to_string(signalCode) + ": " + exePath.string());
        }

        Logger::Log(LogLevel::Debug, "Command finished");
    }
#endif
}

bool ClangCompiler::CompileObjects(const CompileInfo& compileInfo) const
{
    const fs::path objDir = compileInfo.objectOutputPath;
    fs::create_directories(objDir);
    fs::create_directories(compileInfo.buildOutputPath);

    const std::vector<fs::path> cacheFiles = fileChangeManager
        ? fileChangeManager->GetCacheFiles(compileInfo.buildOutputPath, compileInfo.dependencyBuildOutputs)
        : std::vector<fs::path>{};

    bool bNothingChanged = true;

    auto compileSingleFile = [&](const fs::path& file, unsigned int language, const fs::path& compilerPath)
    {
        auto args = CreateCompileArgs(compileInfo, language);

        args.push_back("-c");
        args.push_back(file.string());

        fs::path objFile = objDir / file.filename();
        objFile.replace_extension(objExt);

        args.push_back("-o");
        args.push_back(objFile.string());

        CompileCommandsExporter::Append(compileInfo.buildOutputPath, fs::current_path(), file, args);

        bool needsRebuild = !fs::exists(objFile);
        std::vector<fs::path> dependencyFiles;

        if (fileChangeManager)
        {
            dependencyFiles = CollectFileDependencies(file, compileInfo.includesPaths);

            if (!needsRebuild)
            {
                needsRebuild = fileChangeManager->NeedsRebuild(
                    MakeFileId(file),
                    BuildDependencyIds(dependencyFiles),
                    cacheFiles);
            }
        }

        if (!needsRebuild)
        {
            Logger::Log(LogLevel::Debug, "Skipping unchanged file: %s", file.string().c_str());
            return;
        }

        bNothingChanged = false;
        ExecuteCommand(compilerPath, args);

        if (fileChangeManager)
        {
            const fs::path cacheFilePath = fileChangeManager->GetCacheFilePath(compileInfo.buildOutputPath);
            fileChangeManager->CacheFile(MakeFileId(file), file, cacheFilePath);

            for (const auto& dependency : dependencyFiles)
                fileChangeManager->CacheFile(MakeFileId(dependency), dependency, cacheFilePath);
        }
    };

    for (const auto& file : compileInfo.cFilesToCompile)
        compileSingleFile(file, LANG_C, clangPath);

    for (const auto& file : compileInfo.cppFilesToCompile)
        compileSingleFile(file, LANG_CPP, clangCppPath);

    return bNothingChanged;
}

void ClangCompiler::ArchiveObjects(const ArchiveInfo& archiveInfo) const
{
    fs::path libDir = archiveInfo.libOututPath;

    fs::create_directories(libDir);

    std::vector<std::string> arArgs;
    arArgs.push_back(arPath.string());

#ifdef _WIN32
    arArgs.push_back("rcs");
    arArgs.push_back((libDir / ("lib" + archiveInfo.outputName + libExt)).string());
#else
    arArgs.push_back("rcs");
    arArgs.push_back((libDir / ("lib" + archiveInfo.outputName + libExt)).string());
#endif

    for (const auto& obj : archiveInfo.objects)
        arArgs.push_back(obj.string());

    ExecuteCommand(arPath, arArgs);
}

void ClangCompiler::LinkBinary(const BinaryInfo& linkInfo) const 
{
    std::vector<std::string> argStrings;

    argStrings.push_back(clangCppPath.string());
    
    // Add debug info flag

    if (linkInfo.binaryType == BinaryType::DynamicLibrary)
    {
        argStrings.push_back("-shared");
        argStrings.push_back("-fPIC");
    }   

    // TODO: Add dynamic export (-rdynamic)

    if(linkInfo.bAddDebugInfo)
        argStrings.push_back("-g");

    switch (linkInfo.optimisation)
    {
    case CompilationOptimisation::MIN_SIZE:
        argStrings.push_back("-s");
        argStrings.push_back("-O2");
        argStrings.push_back("-Wl,--gc-sections");
        break;

    case CompilationOptimisation::AGGRESSIVE:
        argStrings.push_back("-O3");
        argStrings.push_back("-flto");
        argStrings.push_back("-Wl,-O1");
        break;

    case CompilationOptimisation::STANDARD:
        argStrings.push_back("-O2");
        break;

    case CompilationOptimisation::FAST:
        argStrings.push_back("-O3");
        argStrings.push_back("-flto");
        argStrings.push_back("-march=native");
        argStrings.push_back("-fuse-ld=lld");
        argStrings.push_back("-Wl,--gc-sections");
        break;

    case CompilationOptimisation::NONE:
        break;
    }

#ifdef _WIN32
    // Static libs (.lib) : full path
    /*
    for (const auto& libPath : compileInfo.staticLibs)
    {
        fs::path fullLibPath = libPath;
        if (fullLibPath.extension() != ".lib")
        {
            fullLibPath = libPath.parent_path() / ("lib" + libPath.filename().string() + ".lib");
        }
        argStrings.push_back(fullLibPath.string());
    }

    fs::path outputFile = compileInfo.buildOutputPath / "bin" / (compileInfo.outputName + ".exe");*/
#else
    for (const auto& path : linkInfo.libPaths)
        argStrings.push_back("-L" + path.string());

    const bool linkingShared = (linkInfo.binaryType == BinaryType::DynamicLibrary);

    // Use $ORIGIN without shell quotes so the linker encodes a relative rpath
    argStrings.push_back("-Wl,-rpath,$ORIGIN");

    // TODO: Add custom dll directory

    if (linkingShared)
        argStrings.push_back("-Wl,--whole-archive");

    for (const auto& group : linkInfo.staticLibsToLink)
    {
        if (group.size() == 1) // Single if no circular dependencies
        {
            argStrings.push_back("-l" + group[0].stem().string());
            continue;
        }

        argStrings.push_back("-Wl,--start-group"); 

        for(const auto& libPath : group)
            argStrings.push_back("-l" + libPath.stem().string());

        argStrings.push_back("-Wl,--end-group");
    }

    if (linkingShared)
        argStrings.push_back("-Wl,--no-whole-archive");

    for (const auto& lib : linkInfo.dynamicLibsToLink)
        argStrings.push_back("-l" + lib.stem().string());
#endif

    fs::path outputFile;

    if (linkInfo.binaryType == BinaryType::DynamicLibrary)
    {
        outputFile = linkInfo.binaryOutputPath / ("lib" + linkInfo.outputName);
        outputFile.replace_extension(".so");
    }
    else
        outputFile = linkInfo.binaryOutputPath / linkInfo.outputName;

    fs::create_directories(outputFile.parent_path());

    argStrings.push_back("-o");
    argStrings.push_back(outputFile.string());

    ExecuteCommand(clangCppPath, argStrings);
}


std::vector<std::string> ClangCompiler::CreateCompileArgs(const CompileInfo& compileInfo, unsigned int fileLanguage) const
{
    std::vector<std::string> args;

    switch (fileLanguage)
    {
    case LANG_C:
        args.push_back(clangPath.string());
        args.push_back(GetCVersionClangOption(compileInfo.cVersion));
        break;
    case LANG_CPP:
        args.push_back(clangCppPath.string());
        args.push_back(GetCppVersionClangOption(compileInfo.cppVersion));
        break;
    default:
        throw std::runtime_error("Failed to compile file, unknown file type.");
    }
        
    // Add debug info flag
    if (compileInfo.bAddDebugInfo)
        args.push_back(GetDebugInfoClangOption(compileInfo.bAddDebugInfo));
    
    switch (compileInfo.optimisation)
    {
        case CompilationOptimisation::MIN_SIZE:
            args.push_back("-Os");
            args.push_back("-ffunction-sections");
            args.push_back("-fdata-sections");
            break;

        case CompilationOptimisation::AGGRESSIVE:
            args.push_back("-O3");
            args.push_back("-flto");
            args.push_back("-funroll-loops");
            args.push_back("-fomit-frame-pointer"); 
            break;

        case CompilationOptimisation::STANDARD:
            args.push_back("-O2");
            break;

        case CompilationOptimisation::FAST:
            args.push_back("-O3");
            args.push_back("-march=native");
            args.push_back("-flto");
            args.push_back("-funroll-loops");
            args.push_back("-fomit-frame-pointer"); 
            break;

        case CompilationOptimisation::NONE:
            args.push_back("-O0");
            break;
    }

        // Add floating point flag
    args.push_back(GetFloatingPointClangOption(compileInfo.floatingPointModel));

    for (const auto& include : compileInfo.includesPaths)
    args.push_back("-I" + include.string());

    for (const auto& macro : compileInfo.macros)
    {
        if (macro.value.has_value())        
            args.push_back("-D" + macro.name + "=" + macro.value.value());
        else
            args.push_back("-D" + macro.name);
    }

    return args;
}

std::string ClangCompiler::GetCVersionClangOption(CVersion version) const
{
    std::string opt("-std=");
    switch (version)
    {
    case CVersion::C90:
        return opt + "c90";
    case CVersion::C99:
        return opt + "c99";
    case CVersion::C11:
        return opt + "c11";
    case CVersion::C17:
        return opt + "c17";
    case CVersion::C23:
        return opt + "c23";
    default:
        return opt + "c17";
    }
}

std::string ClangCompiler::GetCppVersionClangOption(CppVersion version) const
{
    std::string opt("-std=");
    switch (version)
    {
    case CppVersion::CPP98:
        return opt + "c++98";
    case CppVersion::CPP03:
        return opt + "c++03";
    case CppVersion::CPP11:
        return opt + "c++11";
    case CppVersion::CPP14:
        return opt + "c++14";
    case CppVersion::CPP17:
        return opt + "c++17";
    case CppVersion::CPP20:
        return opt + "c++20";
    case CppVersion::CPP23:
        return opt + "c++23";
    case CppVersion::CPP26:
        return opt + "c++2c";
    default:
        return opt + "c++20";
    }
}

std::string ClangCompiler::GetFloatingPointClangOption(FloatingPointModel floatingPoint) const
{
    switch (floatingPoint)
    {
    case FloatingPointModel::STRICT:
        return "-fno-fast-math";
    case FloatingPointModel::PRECISE:
        return "-fno-fast-math";
    case FloatingPointModel::FAST:
        return "-ffast-math";
    default:
        return "-fno-fast-math";
    }
}

std::string ClangCompiler::GetDebugInfoClangOption(bool bAddDebugInfo) const
{
    if (bAddDebugInfo)
        return "-g";
    return "";
}
