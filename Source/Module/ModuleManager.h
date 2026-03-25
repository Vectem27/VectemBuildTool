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

    virtual std::vector<std::string> GetModuleNames() const override
    {
        std::vector<std::string> names;
        for (const auto& pair : modulesStructure)
            names.push_back(pair.first);
        return names;
    }

private:
    std::unordered_map<std::string, ModuleStructureInfo> modulesStructure;

    mutable std::unordered_map<std::string, ModuleInfo> cachedModuleInfos;
};
