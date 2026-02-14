#include "ModuleInfoReader.h"

#include "IModuleInfoReader.h"
#include "Module/Module.h"

#include <filesystem>
#include <sol/sol.hpp>

namespace fs = std::filesystem;

bool DoesFileExists(const std::string& filePath)
{
    return fs::exists(filePath) && fs::is_regular_file(filePath);
}

ModuleInfo ModuleInfoReader::ReadInfo(const std::string& moduleName, const std::string& moduleRulesField) const 
{
    ModuleInfo res;
    res.name = moduleName;

    try
    {
        // Module table
        sol::optional<sol::table> moduleTableField = lua[moduleRulesField].get<sol::optional<sol::table>>();
        if (!moduleTableField) 
            throw ModuleInfoReaderException("Module table is missing : '" + moduleRulesField + "'."); 

        sol::table moduleTable = moduleTableField.value();

        // Module private include directories
        sol::optional<sol::table> publicIncludeDirsTable = moduleTable["PublicIncludeDirectories"];
        if (publicIncludeDirsTable) 
        {
            sol::table publicIncludeDirs = publicIncludeDirsTable.value();

            for (std::size_t i = 1; i <= publicIncludeDirs.size(); ++i)
                res.publicIncludeDirectories.push_back(publicIncludeDirs[i]);
        }

        // Module private include directories
        sol::optional<sol::table> privateIncludeDirsTable = moduleTable["PrivateIncludeDirectories"];
        if (privateIncludeDirsTable) 
        {
            sol::table privateIncludeDirs = privateIncludeDirsTable.value();

            for (std::size_t i = 1; i <= privateIncludeDirs.size(); ++i)
                res.privateIncludeDirectories.push_back(privateIncludeDirs[i]);
        }
        
        // Module private dependendies
        sol::optional<sol::table> publicModuleDependenciesTable = moduleTable["PublicDependencies"];
        if (publicModuleDependenciesTable) 
        {
            sol::table publicModuleDependencies = publicModuleDependenciesTable.value();

            for (std::size_t i = 1; i <= publicModuleDependencies.size(); ++i)
                res.publicModuleDependencies.push_back(publicModuleDependencies[i]);
        }

        // Module private dependendies
        sol::optional<sol::table> privateModuleDependenciesTable = moduleTable["PrivateDependencies"];
        if (privateModuleDependenciesTable) 
        {
            sol::table privateModuleDependencies = privateModuleDependenciesTable.value();

            for (std::size_t i = 1; i <= privateModuleDependencies.size(); ++i)
                res.privateModuleDependencies.push_back(privateModuleDependencies[i]);
        }

        // code dir
        sol::optional<std::string> codeDirField = moduleTable["CodeDir"];
        if (!codeDirField)
            throw ModuleInfoReaderException("Module 'CodeDir' field is missing for '" + moduleName + "'."); 
        
        res.codeDir = codeDirField.value();

        // Module linking type
        sol::optional<std::string> linkingTypeField = moduleTable["LinkingType"];
        if (!linkingTypeField)
            throw ModuleInfoReaderException("Module 'LinkingType' field is missing for '" + moduleName + "'."); 

        if (linkingTypeField.value() == "Static")
            res.linkingType = LinkingType::STATIC;
        else if (linkingTypeField.value() == "Dynamic")
            res.linkingType = LinkingType::DYNAMIC;
        else if (linkingTypeField.value() == "DynamicLoading")
            res.linkingType = LinkingType::DYNAMIC_LOADING;
        else
            throw ModuleInfoReaderException("Invalid linking type for " + moduleName + " : '" + linkingTypeField.value() + "'.");
    }
    catch (const std::exception& e)
    {
        throw ModuleInfoReaderException(e.what());
    }
    catch (...)
    {
        throw ModuleInfoReaderException("Unkown exception");
    }

    return res;
}
