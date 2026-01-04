#pragma once

#include <unordered_map>

#include "IModuleManager.h"

class ModuleManager : public IModuleManager
{
public:
    ModuleManager(bool bCacheModuleInfo = true) : bCacheModuleInfo(bCacheModuleInfo) {}

    virtual void AddModule(std::string moduleName, std::filesystem::path moduleDir) override;

    virtual ModuleInfo ResolveModuleInfo(std::string moduleName, IModuleInfoReader& moduleReader) const override;

private:
    std::unordered_map<std::string, std::filesystem::path> modulesDirectories;

    bool bCacheModuleInfo;
    mutable std::unordered_map<std::string, ModuleInfo> cachedModuleInfos;
};
