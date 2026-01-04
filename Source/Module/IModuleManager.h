#pragma once

#include <filesystem>
#include <stdexcept>

#include "Module.h"

class IModuleInfoReader;

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
    
    virtual void AddModule(std::string moduleName, std::filesystem::path moduleDir) = 0;

    virtual ModuleInfo ResolveModuleInfo(std::string moduleName, IModuleInfoReader& moduleReader) const = 0;
};