#pragma once

#include "Module/IModuleDependencySorter.h"
#include "ModuleManager.h"

struct ModuleDependency
{
    std::string moduleName, depModuleName;
};


class ModuleGraphDependencySorter : public IModuleDependencySorter
{
public:
    ModuleGraphDependencySorter(const ModuleManager& moduleManager) : moduleManager(moduleManager) {}
    virtual SortedModulesGroups Sort(std::vector<std::string> modulesName) const override;

protected:
    void AddModuleAndDependencies(const std::string& moduleName, std::list<std::string>& modules,
                                  std::list<ModuleDependency>& dependancies, const std::vector<std::string>& unitModules) const;
    bool AddModule(const std::string& moduleName, std::list<std::string>& modules) const;
    bool AddDependency(const std::string& moduleName, const std::string& depModuleName,
                       std::list<ModuleDependency>& dependancies) const;

    bool IsUnitModule(const std::string& moduleName, const std::vector<std::string>& unitModules) const;

private:
    const ModuleManager& moduleManager;
};