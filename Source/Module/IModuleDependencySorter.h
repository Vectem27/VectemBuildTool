#pragma once

#include <list>
#include <vector>
#include <string>

using SortedModulesGroups = std::list<std::vector<std::string>>;


class IModuleDependencySorter
{
public:
    virtual ~IModuleDependencySorter() = default;

    virtual SortedModulesGroups Sort(std::vector<std::string> modulesName) const = 0;
};