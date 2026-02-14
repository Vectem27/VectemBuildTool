
#include "ModuleManager.h"

#include "IModuleInfoReader.h"
#include "Module/IModuleManager.h"
#include "Module/Module.h"
#include <exception>
#include <string>


void ModuleManager::AddModule(const std::string& moduleName, ModuleStructureInfo moduleStructure, const std::string& moduleRulesField, IModuleInfoReader& moduleReader) 
{
    modulesStructure[moduleName] = moduleStructure;

    ModuleInfo res;

    try 
    {
        res = moduleReader.ReadInfo(moduleName, moduleRulesField);
        res.directory = moduleStructure.rootDir;
        cachedModuleInfos[moduleName] = res;
    } 
    catch (std::exception e) 
    {
        throw ModuleManagerException( e.what());
    }

}

ModuleInfo ModuleManager::ResolveModuleInfo(const std::string& moduleName) const 
{
    auto it = cachedModuleInfos.find(moduleName);
    if (it != cachedModuleInfos.end())
        return it->second;

    throw ModuleManagerException("Module manager : unknown module : '" + moduleName + "'.");
}
