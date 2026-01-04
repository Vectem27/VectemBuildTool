#include "ModuleInfoReader.h"

#include "IModuleInfoReader.h"

#include <filesystem>
#include <sol/sol.hpp>

namespace fs = std::filesystem;

bool DoesFileExists(const std::string& filePath)
{
    return fs::exists(filePath) && fs::is_regular_file(filePath);
}

ModuleInfo ModuleInfoReader::ReadInfo(std::string filePath) const 
{
    if (!DoesFileExists(filePath))
        throw ModuleInfoReaderException("File does not exists : '" + filePath +"'.");

    ModuleInfo res;

    // Lua
    {
        // Execute script
        {
            sol::protected_function_result result; 
            result = lua.safe_script_file(filePath, sol::load_mode::text);
            if (!result.valid())
            {
                sol::error err = result;
                throw ModuleInfoReaderException("Lua error :\n" + std::string(err.what()));
            }
        }

        // Module table
        sol::table moduleTable = lua["Module"];
        if (!moduleTable.valid()) 
            throw ModuleInfoReaderException("Module table is missing"); 

        // Module name
        sol::optional<std::string> moduleNameField = moduleTable["Name"];
        if (!moduleNameField) 
            throw ModuleInfoReaderException("Module name is not set");
        res.name = moduleNameField.value();

        // Module public include directories
        sol::table publicIncludeDirsTable = moduleTable["PublicIncludeDirectories"];
        if (!publicIncludeDirsTable.valid()) 
            throw ModuleInfoReaderException("'PublicIncludeDirectories' field is not set inside the 'Module' table.");
        
        for (std::size_t i = 1; i <= publicIncludeDirsTable.size(); ++i)
            res.publicIncludeDirectories.push_back(publicIncludeDirsTable[i]);
    }
    return res;
}
