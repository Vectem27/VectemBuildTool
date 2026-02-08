#pragma once

#include <unordered_map>

#include "IModuleManager.h"
#include "Module/Module.h"

class ModuleManager : public IModuleManager
{
public:
    ModuleManager(bool bCacheModuleInfo = true) : bCacheModuleInfo(bCacheModuleInfo) {}

    virtual void AddModule(std::string moduleName, ModuleStructureInfo moduleStructure) override;

    virtual ModuleInfo ResolveModuleInfo(std::string moduleName, IModuleInfoReader& moduleReader) const override;

private:
    std::unordered_map<std::string, ModuleStructureInfo> modulesStructure;

    bool bCacheModuleInfo;
    mutable std::unordered_map<std::string, ModuleInfo> cachedModuleInfos;
};
