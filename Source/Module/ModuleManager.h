#pragma once

#include <unordered_map>

#include "IModuleManager.h"
#include "Module/Module.h"

class ModuleManager : public IModuleManager
{
public:
    ModuleManager() = default;

    virtual void AddModule(const std::string& moduleName, ModuleStructureInfo moduleStructure, const std::string& moduleRulesField, IModuleInfoReader& moduleReader) override;

    virtual ModuleInfo ResolveModuleInfo(const std::string& moduleName) const override;

private:
    std::unordered_map<std::string, ModuleStructureInfo> modulesStructure;

    mutable std::unordered_map<std::string, ModuleInfo> cachedModuleInfos;
};
