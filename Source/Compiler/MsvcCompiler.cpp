#include "MsvcCompiler.h"

#if defined(_WIN32) || defined(_WIN64)

#include <filesystem>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <windows.h>

#include "Core/Logger.hpp"

namespace fs = std::filesystem;

static std::string QuoteIfNeeded(const std::string& s)
{
    if (s.find(' ') != std::string::npos)
        return "\"" + s + "\"";
    return s;
}

static void ExecuteCommand(const fs::path& exePath, const std::vector<std::string>& argStrings)
{
    std::stringstream ss;
    ss << "Starting command : ";
    for (const auto& arg : argStrings)
        ss << " " << arg;
    Logger::Log(LogLevel::Debug, "%s", ss.str().c_str());

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
}

void MsvcCompiler::CompileExecutable(const ExecutableCompileInfo& compileInfo) const
{
    std::vector<std::string> argStrings;

    fs::path msvcPath = "cl.exe";

    argStrings.push_back(msvcPath.string());
    
    // Add C++ version flag
    argStrings.push_back(GetCppVersionMsvcOption(compileInfo.cppVersion));
    
    // Add debug info flag
    if (compileInfo.bAddDebugInfo)
        argStrings.push_back(GetDebugInfoMsvcOption(compileInfo.bAddDebugInfo));
    
    // Add optimization flag
    argStrings.push_back(GetOptimisationMsvcOption(compileInfo.optimisation));
    
    // Add floating point flag
    argStrings.push_back(GetFloatingPointMsvcOption(compileInfo.floatingPointModel));

    for (const auto& include : compileInfo.includesPaths)
        argStrings.push_back("/I" + include.string());

    for (const auto& file : compileInfo.filesToCompile)
        argStrings.push_back(file.string());

    // Static libs (.lib) : full path
    for (const auto& group : compileInfo.staticLibsToLink)
    {
        for (const auto& libPath : group)
        {
            fs::path fullLibPath = libPath;
            if (fullLibPath.extension() != ".lib")
            {
                fullLibPath = libPath.parent_path() / (libPath.filename().string() + ".lib");
            }
            argStrings.push_back(fullLibPath.string());
        }
    }

    fs::path outputFile = compileInfo.buildOutputPath / "bin" / (compileInfo.outputName + ".exe");

    fs::create_directories(outputFile.parent_path());

    argStrings.push_back("/Fe" + outputFile.string());

    ExecuteCommand(msvcPath, argStrings);
}

void MsvcCompiler::CompileLibrary(const LibraryCompileInfo& compileInfo) const
{
    fs::path msvcPath = "cl.exe";
    fs::path libPath = "lib.exe";

    fs::path objDir = compileInfo.buildOutputPath / "obj";
    fs::path libDir = compileInfo.buildOutputPath / "lib";

    fs::create_directories(objDir);
    fs::create_directories(libDir);

    std::vector<fs::path> objects;

    for (const auto& file : compileInfo.filesToCompile)
    {
        std::vector<std::string> args;
        args.push_back(msvcPath.string());
        
        // Add C++ version flag
        args.push_back(GetCppVersionMsvcOption(compileInfo.cppVersion));
        
        // Add debug info flag
        if (compileInfo.bAddDebugInfo)
            args.push_back(GetDebugInfoMsvcOption(compileInfo.bAddDebugInfo));
        
        // Add optimization flag
        args.push_back(GetOptimisationMsvcOption(compileInfo.optimisation));
        
        // Add floating point flag
        args.push_back(GetFloatingPointMsvcOption(compileInfo.floatingPointModel));

        for (const auto& include : compileInfo.includesPaths)
            args.push_back("/I" + include.string());

        args.push_back("/c");
        args.push_back(file.string());

        fs::path objFile = objDir / file.filename();
        objFile.replace_extension(".obj");

        args.push_back("/Fo" + objFile.string());

        objects.push_back(objFile);
        ExecuteCommand(msvcPath, args);
    }

    std::vector<std::string> libArgs;
    libArgs.push_back(libPath.string());

    for (const auto& obj : objects)
        libArgs.push_back(obj.string());

    libArgs.push_back("/OUT:" + (libDir / (compileInfo.outputName + ".lib")).string());

    ExecuteCommand(libPath, libArgs);
}

std::string MsvcCompiler::GetCVersionMsvcOption(CVersion version) const
{
    // MSVC doesn't have direct C version flags, but we can specify through /std
    switch (version)
    {
    case CVersion::C90:
    case CVersion::C99:
    case CVersion::C11:
    case CVersion::C17:
    case CVersion::C23:
    default:
        return "/std:c17";  // Default for MSVC
    }
}

std::string MsvcCompiler::GetCppVersionMsvcOption(CppVersion version) const
{
    switch (version)
    {
    case CppVersion::CPP98:
    case CppVersion::CPP03:
    case CppVersion::CPP11:
    case CppVersion::CPP14:
        return "/std:c++14";
    case CppVersion::CPP17:
        return "/std:c++17";
    case CppVersion::CPP20:
        return "/std:c++20";
    case CppVersion::CPP23:
        return "/std:c++latest";
    case CppVersion::CPP26:
        return "/std:c++latest";
    default:
        return "/std:c++20";
    }
}

std::string MsvcCompiler::GetOptimisationMsvcOption(CompilationOptimisation optimisation) const
{
    switch (optimisation)
    {
    case CompilationOptimisation::NONE:
        return "/Od";
    case CompilationOptimisation::STANDARD:
        return "/O2";
    case CompilationOptimisation::AGGRESSIVE:
        return "/O3";
    case CompilationOptimisation::FAST:
        return "/O3";
    case CompilationOptimisation::MIN_SIZE:
        return "/Os";
    default:
        return "/O2";
    }
}

std::string MsvcCompiler::GetFloatingPointMsvcOption(FloatingPointModel floatingPoint) const
{
    switch (floatingPoint)
    {
    case FloatingPointModel::STRICT:
        return "/fp:strict";
    case FloatingPointModel::PRECISE:
        return "/fp:precise";
    case FloatingPointModel::FAST:
        return "/fp:fast";
    default:
        return "/fp:precise";
    }
}

std::string MsvcCompiler::GetDebugInfoMsvcOption(bool bAddDebugInfo) const
{
    if (bAddDebugInfo)
        return "/Zi";
    return "";
}

#endif