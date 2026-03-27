#pragma once

#include <filesystem>
#include <vector>
#include <list>

#include "Compilation.h"

/**
 * @brief The library linking type, either static or dynamic
 */
enum class LinkType
{
    STATIC,
    DYNAMIC
};

/**
 * @brief Compilation basics information for an executable or a library
 */
struct CompileInfo
{
    std::string outputName;
    std::filesystem::path buildOutputPath;
    std::vector<std::filesystem::path> filesToCompile;
    std::vector<std::filesystem::path> includesPaths;

    bool bAddDebugInfo = false;
    CVersion cVersion = CVersion::C17;
    CppVersion cppVersion = CppVersion::CPP20;
    int supportedPlatforms = 0xFF; // All platforms by default
    CompilationOptimisation optimisation = CompilationOptimisation::STANDARD;
    FloatingPointModel floatingPointModel = FloatingPointModel::PRECISE;
    
    std::vector<std::filesystem::path> staticLibPaths;

    std::list<std::vector<std::filesystem::path>> staticLibsToLink;
    std::vector<std::filesystem::path> dynamicLibsToLink;
};

/**
 * @brief Executable compilation information, including the static libraries to link against
 */
struct ExecutableCompileInfo : CompileInfo
{
    std::vector<std::filesystem::path> libPaths;
};

/**
 * @brief Library compilation information, including the link type (static or dynamic)
 */
struct LibraryCompileInfo : CompileInfo
{
    LinkType linkType;
};

/**
 * @brief The interface for a compiler that can compile executables and libraries.
 */
class ICompiler
{
public:
    virtual ~ICompiler() = default;

    /**
     * @brief Compiles an executable with the given compilation information.
     *
     * @param compileInfo The compilation information for the executable to compile.
     */
    virtual void CompileExecutable(const ExecutableCompileInfo& compileInfo) const = 0;

    /**
     * @brief Compiles a library with the given compilation information.
     *
     * @param compileInfo The compilation information for the library to compile.
     */
    virtual void CompileLibrary(const LibraryCompileInfo& compileInfo) const = 0;
};

/**
 * @brief The interface for a compiler factory that can create compilers.
 *
 * This class implement the abstract factory design pattern.
 */
class ICompilerFactory
{
public:
    virtual ~ICompilerFactory() = default;

    /**
     * @brief Creates a compiler instance.
     *
     * @return A pointer to the created compiler instance.
     */
    virtual ICompiler* Create() const = 0;
};
