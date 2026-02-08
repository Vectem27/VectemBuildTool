#pragma once

#include <filesystem>
#include <stdexcept>

#include "Module.h"

class IModuleInfoReader;

/**
 * @brief Structure representing the files structure of a module
 */
struct ModuleStructureInfo
{
    std::filesystem::path rootDir;
    std::filesystem::path buildRulesFile;

    /**
     * @brief Directory containing the code, relative to the module root.
     */
    std::filesystem::path codeDir;
};

class ModuleManagerException : public std::runtime_error
{
public:
    explicit ModuleManagerException(const std::string& message) : std::runtime_error(message)
    {
    }
};

class IModuleManager
{
public:
    virtual ~IModuleManager() = default;
    
    virtual void AddModule(std::string moduleName, ModuleStructureInfo moduleStructure) = 0;

    virtual ModuleInfo ResolveModuleInfo(std::string moduleName, IModuleInfoReader& moduleReader) const = 0;
};