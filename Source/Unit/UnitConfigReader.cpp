#include "UnitConfigReader.h"
#include "Unit/IUnitConfigReader.h"

#include <exception>
#include <sol/optional_implementation.hpp>
#include <sol/sol.hpp>
#include <sol/table.hpp>

std::vector<UnitInfo> UnitConfigReader::ReadUnitsConfig(std::filesystem::path unitRoot) const
{
    std::vector<UnitInfo> res;

    try
    {

        // Module table
        sol::table unitsConfigTable = lua["UnitsConfig"];
        if (!unitsConfigTable.valid())
            throw UnitConfigReaderException("Units config table is missing");

        for (const auto& pair : unitsConfigTable)
        {
            UnitInfo info;
        
            sol::object key = pair.first;
            sol::object value = pair.second;

            // Name
            if (!key.is<std::string>())
                throw UnitConfigReaderException("Unit config key is not a string.");

            info.name = key.as<std::string>();

            // infos

            if (!value.is<sol::table>())
                throw UnitConfigReaderException("Unit config table is not a table for '" + 
                                                    info.name + "'.");

            auto infoTable = value.as<sol::table>();

            // Module dir
            sol::optional<std::vector<std::string>> modulesDirField;
            modulesDirField = infoTable["ModulesDir"].get<sol::optional<std::vector<std::string>>>();
            if (!modulesDirField)
                throw UnitConfigReaderException("Unit config 'ModulesDir' array is missing for '" + 
                                                    info.name + "'.");

            for (const auto& dir : modulesDirField.value())
                info.modulesDirs.emplace_back(std::move(unitRoot / dir));

            // Target dir
            sol::optional<std::string> targetsDirField;
            targetsDirField = infoTable["TargetsDir"].get<sol::optional<std::string>>();
            if (targetsDirField)
                info.targetsDir = unitRoot / targetsDirField.value();

            // Build dir
            sol::optional<std::string> buildDirField;
            buildDirField = infoTable["BuildDir"].get<sol::optional<std::string>>();
            if (!buildDirField)
                throw UnitConfigReaderException("Unit config 'BuildDir' field is missing for '" + 
                                                    info.name + "'.");
            info.buildDir = unitRoot / buildDirField.value();

            res.emplace_back(std::move(info));
        }

    }
    catch (const std::exception& e)
    {
        throw UnitConfigReaderException(e.what());
    }
    catch (...)
    {
        throw UnitConfigReaderException("Unkown exception");
    }

    return res;
}
