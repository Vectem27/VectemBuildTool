
#include "ModuleManager.h"

#include "IModuleInfoReader.h"
#include "Module/IModuleManager.h"
#include "Module/Module.h"
#include <exception>
#include <string>


void ModuleManager::AddModule(std::string moduleName, std::filesystem::path moduleDir) 
{
    modulesDirectories[moduleName] = moduleDir;
}

ModuleInfo ModuleManager::ResolveModuleInfo(std::string moduleName, IModuleInfoReader& moduleReader) const 
{
    if (bCacheModuleInfo)
    {
        auto it = cachedModuleInfos.find(moduleName);
        if (it != cachedModuleInfos.end())
            return it->second;
    }

    ModuleInfo res;

    try 
    {
        res = moduleReader.ReadInfo(modulesDirectories.at(moduleName) / "Module.lua");
    } 
    catch (std::exception e) 
    {
        throw ModuleManagerException( e.what());
    }

    if (bCacheModuleInfo)
        cachedModuleInfos[moduleName] = res;
    return res;
}
