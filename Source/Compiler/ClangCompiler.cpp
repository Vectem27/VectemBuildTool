#include "ClangCompiler.h"

#include <filesystem>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <sstream>

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

void ClangCompiler::CompileExecutable(const ExecutableCompileInfo& compileInfo) const
{
    std::vector<std::string> argStrings;

#ifdef _WIN32
    fs::path clangPath = "clang++.exe";
#else
    fs::path clangPath = "clang++";
#endif

    argStrings.push_back(clangPath.string());
    
    // Add C version flag
    if (compileInfo.cVersion != CVersion::C17)
        argStrings.push_back(GetCVersionClangOption(compileInfo.cVersion));
    
    // Add C++ version flag
    argStrings.push_back(GetCppVersionClangOption(compileInfo.cppVersion));
    
    // Add debug info flag
    if (compileInfo.bAddDebugInfo)
        argStrings.push_back(GetDebugInfoClangOption(compileInfo.bAddDebugInfo));
    
    // Add optimization flag
    argStrings.push_back(GetOptimisationClangOption(compileInfo.optimisation));
    
    // Add floating point flag
    argStrings.push_back(GetFloatingPointClangOption(compileInfo.floatingPointModel));

    for (const auto& include : compileInfo.includesPaths)
        argStrings.push_back("-I" + include.string());

    for (const auto& file : compileInfo.filesToCompile)
        argStrings.push_back(file.string());

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
    for (const auto& path : compileInfo.staticLibPaths)
            argStrings.push_back("-L" + path.string());

    for (const auto& group : compileInfo.staticLibsToLink)
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

    //for (const auto& libPath : compileInfo.staticLibs)
    //    argStrings.push_back("-l" + libPath.stem().string());

    fs::path outputFile = compileInfo.buildOutputPath / "bin" / compileInfo.outputName;
#endif

    fs::create_directories(outputFile.parent_path());

    argStrings.push_back("-o");
    argStrings.push_back(outputFile.string());

    ExecuteCommand(clangPath, argStrings);
}

void ClangCompiler::CompileLibrary(const LibraryCompileInfo& compileInfo) const
{
#ifdef _WIN32
    fs::path clangPath = "clang++.exe";
    fs::path arPath = "llvm-ar.exe";
    std::string objExt = ".obj";
    std::string libExt = ".lib";
#else
    fs::path clangPath = "clang++";
    fs::path arPath = "ar";
    std::string objExt = ".o";
    std::string libExt = ".a";
#endif

    fs::path objDir = compileInfo.buildOutputPath / "obj";
    fs::path libDir = compileInfo.buildOutputPath / "lib";

    fs::create_directories(objDir);
    fs::create_directories(libDir);

    std::vector<fs::path> objects;

    for (const auto& file : compileInfo.filesToCompile)
    {
        std::vector<std::string> args;
        args.push_back(clangPath.string());
        
        // Add C version flag
        if (compileInfo.cVersion != CVersion::C17)
            args.push_back(GetCVersionClangOption(compileInfo.cVersion));
        
        // Add C++ version flag
        args.push_back(GetCppVersionClangOption(compileInfo.cppVersion));
        
        // Add debug info flag
        if (compileInfo.bAddDebugInfo)
            args.push_back(GetDebugInfoClangOption(compileInfo.bAddDebugInfo));
        
        // Add optimization flag
        args.push_back(GetOptimisationClangOption(compileInfo.optimisation));
        
        // Add floating point flag
        args.push_back(GetFloatingPointClangOption(compileInfo.floatingPointModel));

        for (const auto& include : compileInfo.includesPaths)
            args.push_back("-I" + include.string());

        args.push_back("-c");
        args.push_back(file.string());

        fs::path objFile = objDir / file.filename();
        objFile.replace_extension(objExt);

        args.push_back("-o");
        args.push_back(objFile.string());

        objects.push_back(objFile);
        CompileCommandsExporter::Append(compileInfo.buildOutputPath, fs::current_path(), file, args);
        ExecuteCommand(clangPath, args);
    }

    std::vector<std::string> arArgs;
    arArgs.push_back(arPath.string());

#ifdef _WIN32
    arArgs.push_back("rcs");
    arArgs.push_back((libDir / ("lib" + compileInfo.outputName + libExt)).string());
#else
    arArgs.push_back("rcs");
    arArgs.push_back((libDir / ("lib" + compileInfo.outputName + libExt)).string());
#endif

    for (const auto& obj : objects)
        arArgs.push_back(obj.string());

    ExecuteCommand(arPath, arArgs);
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

std::string ClangCompiler::GetOptimisationClangOption(CompilationOptimisation optimisation) const
{
    switch (optimisation)
    {
    case CompilationOptimisation::NONE:
        return "-O0";
    case CompilationOptimisation::STANDARD:
        return "-O2";
    case CompilationOptimisation::AGGRESSIVE:
        return "-O3";
    case CompilationOptimisation::FAST:
        return "-O3";
    case CompilationOptimisation::MIN_SIZE:
        return "-Os";
    default:
        return "-O2";
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
