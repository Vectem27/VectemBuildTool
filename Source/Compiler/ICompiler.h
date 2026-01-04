#pragma once

#include <filesystem>
#include <vector>

#include "Compilation.h"

enum class LinkType
{
    STATIC,
    DYNAMIC
};

struct CompileInfo
{
    std::string outputName;
    std::filesystem::path buildOutputPath;
    std::vector<std::filesystem::path> filesToCompile;
    std::vector<std::filesystem::path> includesPaths;

    CppVersion cppVersion;
    CompilationOptimisation optimisation;
};

struct ExecutableCompileInfo : CompileInfo
{
    std::vector<std::filesystem::path> staticLibs;
};

struct LibraryCompileInfo : CompileInfo
{
    LinkType linkType;
};

class ICompiler
{
public:
    virtual ~ICompiler() = default;
    virtual void CompileExecutable(const ExecutableCompileInfo& compileInfo) const = 0;
    virtual void CompileLibrary(const LibraryCompileInfo& compileInfo) const = 0;
};

class ICompilerFactory
{
public:
    virtual ~ICompilerFactory() = default;
    virtual ICompiler* Create() const = 0;
};
