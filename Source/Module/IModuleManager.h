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
    
    virtual void AddModule(const std::string& moduleName, ModuleStructureInfo moduleStructure, const std::string& moduleRulesField, IModuleInfoReader& moduleReader) = 0;

    virtual ModuleInfo ResolveModuleInfo(const std::string& moduleName) const = 0;
};