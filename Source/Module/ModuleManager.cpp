
#include "ModuleManager.h"

#include "IModuleInfoReader.h"
#include "Module/IModuleManager.h"
#include "Module/Module.h"
#include <exception>
#include <string>


void ModuleManager::AddModule(const std::string& moduleName, ModuleStructureInfo moduleStructure) 
{
    modulesStructure[moduleName] = moduleStructure;
}

ModuleInfo ModuleManager::ResolveModuleInfo(const std::string& moduleName, const std::string& moduleRulesField, IModuleInfoReader& moduleReader) const 
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
        res = moduleReader.ReadInfo(modulesStructure.at(moduleName).buildRulesFile.string(), moduleRulesField);
    } 
    catch (std::exception e) 
    {
        throw ModuleManagerException( e.what());
    }

    if (bCacheModuleInfo)
        cachedModuleInfos[moduleName] = res;
    return res;
}
