#pragma once

#include "Module/IModuleManager.h"

class IModuleManager;

using IncludesToAdd = std::vector<std::filesystem::path>;

class IModuleIncludeSolver
{
public:
    virtual ~IModuleIncludeSolver() = default;

    virtual IncludesToAdd Resolve(const std::string& modulesName, const IModuleManager& moduleManager) const = 0;
};