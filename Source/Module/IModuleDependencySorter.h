#pragma once

#include <list>

#include "Module/IModuleManager.h"

class IModuleManager;

using SortedModulesGroups = std::list<std::vector<std::string>>;

class IModuleDependencySorter
{
public:
    virtual ~IModuleDependencySorter() = default;

    virtual SortedModulesGroups Sort(std::vector<std::string> modulesName, const IModuleManager& moduleManager) const = 0;
};