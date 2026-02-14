#include "ModuleInfoReader.h"

#include "IModuleInfoReader.h"

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

        // Module public include directories
        sol::table publicIncludeDirsTable = moduleTable["PublicIncludeDirectories"];
        if (!publicIncludeDirsTable.valid()) 
            throw ModuleInfoReaderException("'PublicIncludeDirectories' field is not set inside the 'Module' table.");
        
        for (std::size_t i = 1; i <= publicIncludeDirsTable.size(); ++i)
            res.publicIncludeDirectories.push_back(publicIncludeDirsTable[i]);

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
