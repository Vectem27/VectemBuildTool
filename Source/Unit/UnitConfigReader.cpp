#include "UnitConfigReader.h"
#include "Unit/IUnitConfigReader.h"

#include <exception>
#include <sol/optional_implementation.hpp>
#include <sol/sol.hpp>
#include <sol/table.hpp>

UnitsConfig UnitConfigReader::ReadUnitsConfig(std::filesystem::path unitRoot) const
{
    UnitsConfig res;

    try
    {
        // Units config table
        sol::table unitsConfigTable = lua["UnitsConfig"];
        if (!unitsConfigTable.valid())
            throw UnitConfigReaderException("Units config table is missing");

        // Unit File Name
        sol::optional<std::string> unitFileNameField =
            unitsConfigTable["UnitFileName"].get<sol::optional<std::string>>();
        if (!unitFileNameField)
            throw UnitConfigReaderException("Units config 'UnitFileName' field is missing.");
        res.UnitFileName = unitFileNameField.value();

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
                continue;
                //throw UnitConfigReaderException("Unit config table is not a table for '" + info.name + "'.");

            auto infoTable = value.as<sol::table>();

            // Module dir
            sol::optional<std::vector<std::string>> modulesDirField;
            modulesDirField = infoTable["ModulesDir"].get<sol::optional<std::vector<std::string>>>();
            if (!modulesDirField)
                throw UnitConfigReaderException("Unit config 'ModulesDir' array is missing for '" + info.name + "'.");

            for (const auto& dir : modulesDirField.value())
                info.modulesDirs.emplace_back(std::move(unitRoot / dir));

            // Module Root Name
            sol::optional<std::string> moduleRootNameField =
                infoTable["ModuleRootName"].get<sol::optional<std::string>>();
            if (!moduleRootNameField)
                throw UnitConfigReaderException("Unit config 'ModuleRootName' field is missing for '" + info.name + "'.");
            info.moduleRootName = moduleRootNameField.value();

            // Module File Name
            sol::optional<std::string> moduleFileNameField =
                infoTable["ModuleFileName"].get<sol::optional<std::string>>();
            if (!moduleFileNameField)
                throw UnitConfigReaderException("Unit config 'ModuleFileName' field is missing for '" + info.name + "'.");
            info.moduleFileName = moduleFileNameField.value();

            // Module Class Name
            sol::optional<std::string> moduleClassNameField =
                infoTable["ModuleClassName"].get<sol::optional<std::string>>();
            if (!moduleClassNameField)
                throw UnitConfigReaderException("Unit config 'ModuleClassName' field is missing for '" + info.name + "'.");
            info.moduleClassName = moduleClassNameField.value();

            // Target dir
            sol::optional<std::string> targetsDirField;
            targetsDirField = infoTable["TargetsDir"].get<sol::optional<std::string>>();
            if (targetsDirField)
                info.targetsDir = unitRoot / targetsDirField.value();

            // Build dir
            sol::optional<std::string> buildDirField;
            buildDirField = infoTable["BuildDir"].get<sol::optional<std::string>>();
            if (!buildDirField)
                throw UnitConfigReaderException("Unit config 'BuildDir' field is missing for '" + info.name + "'.");
            info.buildDir = unitRoot / buildDirField.value();

            res.unitsInfo.emplace_back(std::move(info));
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
