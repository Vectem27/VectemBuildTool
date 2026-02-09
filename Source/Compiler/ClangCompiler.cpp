#include "ClangCompiler.h"

#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <vector>
#include <sstream>

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
    std::cout << ss.str() << std::endl;

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
        std::cerr << "Command failed with code " << ret << std::endl;
#else
    std::vector<char*> args;
    for (auto& s : argStrings)
        args.push_back(const_cast<char*>(s.c_str()));
    args.push_back(nullptr);

    pid_t pid = fork();
    if (pid == 0)
    {
        execvp(exePath.c_str(), args.data());
        perror("execvp");
        exit(1);
    }
    else
    {
        wait(nullptr);
        std::cout << "Command finished\n";
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
    argStrings.push_back(GetCppVersionClangOption(compileInfo.cppVersion));

    for (const auto& include : compileInfo.includesPaths)
        argStrings.push_back("-I" + include.string());

    for (const auto& file : compileInfo.filesToCompile)
        argStrings.push_back(file.string());

#ifdef _WIN32
    // Static libs (.lib) : full path
    for (const auto& libPath : compileInfo.staticLibs)
    {
        fs::path fullLibPath = libPath;
        if (fullLibPath.extension() != ".lib")
        {
            fullLibPath = libPath.parent_path() / ("lib" + libPath.filename().string() + ".lib");
        }
        argStrings.push_back(fullLibPath.string());
    }

    fs::path outputFile = compileInfo.buildOutputPath / "bin" / (compileInfo.outputName + ".exe");
#else
    for (const auto& libPath : compileInfo.staticLibs)
        argStrings.push_back("-L" + libPath.parent_path().string());

    for (const auto& libPath : compileInfo.staticLibs)
        argStrings.push_back("-l" + libPath.stem().string());

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
        args.push_back(GetCppVersionClangOption(compileInfo.cppVersion));

        for (const auto& include : compileInfo.includesPaths)
            args.push_back("-I" + include.string());

        args.push_back("-c");
        args.push_back(file.string());

        fs::path objFile = objDir / file.filename();
        objFile.replace_extension(objExt);

        args.push_back("-o");
        args.push_back(objFile.string());

        objects.push_back(objFile);
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

std::string ClangCompiler::GetCppVersionClangOption(CppVersion version) const
{
    std::string opt("-std=");
    switch (version)
    {
    case CppVersion::CPP_11:
        return opt + "c++11";
    case CppVersion::CPP_14:
        return opt + "c++14";
    case CppVersion::CPP_17:
        return opt + "c++17";
    case CppVersion::CPP_20:
        return opt + "c++20";
    case CppVersion::CPP_23:
        return opt + "c++23";
    default:
        return opt + "c++11";
    }
}
