
#include "ModuleManager.h"

#include "IModuleInfoReader.h"
#include "Module/IModuleManager.h"
#include "Module/Module.h"
#include <exception>
#include <string>


void ModuleManager::AddModule(std::string moduleName, ModuleStructureInfo moduleStructure) 
{
    modulesStructure[moduleName] = moduleStructure;
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
        res = moduleReader.ReadInfo(modulesStructure.at(moduleName).buildRulesFile);
    } 
    catch (std::exception e) 
    {
        throw ModuleManagerException( e.what());
    }

    if (bCacheModuleInfo)
        cachedModuleInfos[moduleName] = res;
    return res;
}
