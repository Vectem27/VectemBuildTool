#include "ModuleInfoReader.h"

#include "Core/Logger.hpp"
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

        sol::optional<sol::table> additionalStaticLibDirField = moduleTable["AdditionalStaticLibDirectory"];
        if (additionalStaticLibDirField) 
        {
            sol::table additionalStaticLibDir = additionalStaticLibDirField.value();

            for (std::size_t i = 1; i <= additionalStaticLibDir.size(); ++i)
                res.additionalStaticLibDir.push_back(additionalStaticLibDir[i]);
        }

        sol::optional<sol::table> additionalStaticLibField = moduleTable["AdditionalStaticLib"];
        if (additionalStaticLibField) 
        {
            sol::table additionalStaticLib = additionalStaticLibField.value();

            for (std::size_t i = 1; i <= additionalStaticLib.size(); ++i)
                res.additionalStaticLib.push_back(additionalStaticLib[i]);
        }

        sol::optional<sol::table> additionalMacroField = moduleTable["AdditionalMacro"];
        if (additionalMacroField) 
        {
            sol::table additionalmacro = additionalMacroField.value();

            for (std::size_t i = 1; i <= additionalmacro.size(); ++i)
            {
                Macro macro;
                sol::optional<sol::table> macroField = additionalmacro[i];
                if (!macroField.has_value())
                    throw ModuleInfoReaderException("AdditionalMacro sub field is not a table");
                
                auto macroDef = macroField.value();

                sol::optional<std::string> macroName = macroDef["Name"];
                if (!macroName.has_value())
                    Logger::Log(LogLevel::Warning, "AdditionalMacro sub table is ignored for no name set.");
                macro.name = macroName.value();

                sol::optional<std::string> macroValue = macroDef["Value"];

                macro.value = macroValue.has_value() ? macro.value : std::nullopt;

                res.additionalMacro.push_back(macro);
            }
        }


        
        // code dir
        sol::optional<std::string> codeDirField = moduleTable["CodeDir"];
        if (!codeDirField)
            throw ModuleInfoReaderException("Module 'CodeDir' field is missing for '" + moduleName + "'."); 
        
        res.codeDir = codeDirField.value();
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
