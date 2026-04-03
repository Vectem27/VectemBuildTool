#pragma once

#include <filesystem>
#include <vector>
#include <list>

#include "Compilation.h"

/**
 * @brief Compilation basics information for an executable or a library
 */
struct CompileInfo
{
    std::filesystem::path buildOutputPath;
    std::filesystem::path objectOutputPath;

    std::vector<std::filesystem::path> cppFilesToCompile;
    std::vector<std::filesystem::path> cFilesToCompile;
    std::vector<std::filesystem::path> includesPaths;

    bool bAddDebugInfo = false;
    // TODO: Add language selector
    CVersion cVersion = CVersion::C17; 
    CppVersion cppVersion = CppVersion::CPP20;
    CompilationOptimisation optimisation = CompilationOptimisation::STANDARD;
    FloatingPointModel floatingPointModel = FloatingPointModel::PRECISE;

    // TODO: Adding defines
};

struct ArchiveInfo
{
    std::string outputName;
    std::filesystem::path libOututPath;
    std::vector<std::filesystem::path> objects;
};

enum class BinaryType { Executable, DynamicLibrary };

struct BinaryInfo
{
    BinaryType binaryType;

    std::list<std::vector<std::filesystem::path>> staticLibsToLink;
    std::vector<std::filesystem::path> dynamicLibsToLink;
    std::vector<std::filesystem::path> libPaths;

    std::filesystem::path binaryOutputPath;
    std::string outputName;

    bool bAddDebugInfo;
    CompilationOptimisation optimisation;
};

/**
 * @brief The interface for a compiler that can compile executables and libraries.
 */
class ICompiler
{
public:
    virtual ~ICompiler() = default;

    /**
     * @brief Compiles objects with the given compilation information.
     *
     * @param compileInfo The compilation information for the library to compile.
     */
    virtual void CompileObjects(const CompileInfo& compileInfo) const = 0;

    /**
     * @brief Archive objects into a single library.
     * 
     * @param archiveInfo The archive infromations.
     */
    virtual void ArchiveObjects(const ArchiveInfo& archiveInfo) const = 0;

    /**
     * @brief Link every dependancies to get a dynamic lubrary or an executable.
     *
     * @param linkInfo The linking informations.
     */
    virtual void LinkBinary(const BinaryInfo& linkInfo) const = 0;
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
