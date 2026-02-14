#include "BuildConfigReader.h"
#include "BuildConfig/BuildConfig.h"
#include "IBuildConfigReader.h"

#include <exception>
#include <sol/optional_implementation.hpp>
#include <sol/sol.hpp>
#include <sol/table.hpp>

BuildConfig BuildConfigReader::ReadBuildConfig(std::filesystem::path unitRoot) const
{
    BuildConfig res;

    try
    {
        // Units config table
        sol::optional<sol::table> unitsConfigTableField = lua["BuildConfig"].get<sol::optional<sol::table>>();
        if (!unitsConfigTableField)
            throw BuildConfigReaderException("Units config table is missing");

        sol::table unitsConfigTable = unitsConfigTableField.value();

        for (const auto& pair : unitsConfigTable)
        {
            UnitConfig info;

            sol::object key = pair.first;
            sol::object value = pair.second;

            // Name
            if (!key.is<std::string>())
                throw BuildConfigReaderException("Unit config key is not a string.");

            info.type = key.as<std::string>();

            // infos

            if (!value.is<sol::table>())
                throw BuildConfigReaderException("Unit config table is not a table for '" + info.type + "'.");

            auto infoTable = value.as<sol::table>();

            // Unit File Name
            sol::optional<std::string> unitFileNameField;
            unitFileNameField = infoTable["UnitFileName"].get<sol::optional<std::string>>();
            if (!unitFileNameField)
                throw BuildConfigReaderException("Units config 'UnitFileName' field is missing.");
            info.unitFileName = unitFileNameField.value();

            // Unit File Name
            sol::optional<std::string> unitClassNameField;
            unitClassNameField = infoTable["UnitClassName"].get<sol::optional<std::string>>();
            if (!unitClassNameField)
                throw BuildConfigReaderException("Units config 'UnitClassName' field is missing.");
            info.unitClassName = unitClassNameField.value();

            // Module dir
            sol::optional<std::vector<std::string>> modulesDirField;
            modulesDirField = infoTable["ModulesDir"].get<sol::optional<std::vector<std::string>>>();
            if (!modulesDirField)
                throw BuildConfigReaderException("Unit config 'ModulesDir' array is missing for '" + info.type + "'.");

            for (const auto& dir : modulesDirField.value())
                info.modulesDirs.emplace_back(std::move(unitRoot / dir));

            // Module Root Name
            sol::optional<std::string> moduleRootNameField =
                infoTable["ModuleRootName"].get<sol::optional<std::string>>();
            if (!moduleRootNameField)
                throw BuildConfigReaderException("Unit config 'ModuleRootName' field is missing for '" + info.type + "'.");
            info.moduleRootName = moduleRootNameField.value();

            // Module File Name
            sol::optional<std::string> moduleFileNameField =
                infoTable["ModuleFileName"].get<sol::optional<std::string>>();
            if (!moduleFileNameField)
                throw BuildConfigReaderException("Unit config 'ModuleFileName' field is missing for '" + info.type + "'.");
            info.moduleFileName = moduleFileNameField.value();

            // Module Class Name
            sol::optional<std::string> moduleClassNameField =
                infoTable["ModuleClassName"].get<sol::optional<std::string>>();
            if (!moduleClassNameField)
                throw BuildConfigReaderException("Unit config 'ModuleClassName' field is missing for '" + info.type + "'.");
            info.moduleClassName = moduleClassNameField.value();

            // Target dir (Target info are not mendatory, some subunits doesn't need one)
            sol::optional<std::string> targetsDirField;
            targetsDirField = infoTable["TargetsDir"].get<sol::optional<std::string>>();
            if (targetsDirField)
                info.targetsDir = unitRoot / targetsDirField.value();

            sol::optional<std::string> targetFileNameField;
            targetFileNameField = infoTable["TargetFileName"].get<sol::optional<std::string>>();
            if (targetFileNameField)
                info.targetFileName = targetFileNameField.value();

            sol::optional<std::string> targetClassNameField;
            targetClassNameField = infoTable["TargetClassName"].get<sol::optional<std::string>>();
            if (targetClassNameField)
                info.targetClassName = targetClassNameField.value();

            // Build dir
            sol::optional<std::string> buildDirField;
            buildDirField = infoTable["BuildDir"].get<sol::optional<std::string>>();
            if (!buildDirField)
                throw BuildConfigReaderException("Unit config 'BuildDir' field is missing for '" + info.type + "'.");
            info.buildDir = unitRoot / buildDirField.value();


            // Sub units
            sol::optional<std::vector<sol::table>> subUnitsField;
            subUnitsField = infoTable["SubUnits"].get<sol::optional<std::vector<sol::table>>>();
            if (!subUnitsField)
                throw BuildConfigReaderException("Unit config 'SubUnits' array is missing for '" + info.type + "'.");

            for (const auto& subUnit : subUnitsField.value())
            {
                SubUnitConfig subUnitConfig;

                // Recursive Dir
                sol::optional<bool> recursiveDirField;
                recursiveDirField = subUnit["bRecursive"].get<sol::optional<bool>>();
                if (!recursiveDirField)
                    throw BuildConfigReaderException( "Unit : A sub-unit config 'bRecursive' field is missing for '" + info.type + "'.");
                subUnitConfig.bRecursive = recursiveDirField.value();

                // Dir
                sol::optional<std::string> directoryField;
                directoryField = subUnit["Dir"].get<sol::optional<std::string>>();
                if (!directoryField)
                    throw BuildConfigReaderException( "Unit : A sub-unit config 'Dir' field is missing for '" + info.type + "'.");
                subUnitConfig.dir = directoryField.value();

                // Unit Type
                sol::optional<std::string> subUnitTypeField;
                subUnitTypeField = subUnit["UnitType"].get<sol::optional<std::string>>();
                if (!subUnitTypeField)
                    throw BuildConfigReaderException( "Unit : A sub-unit config 'UnitType' field is missing for '" + info.type + "'.");
                subUnitConfig.unitType = subUnitTypeField.value();

                // Unit Type
                sol::optional<std::string> subUnitRootNameField;
                subUnitRootNameField = subUnit["UnitRootName"].get<sol::optional<std::string>>();
                if (!subUnitRootNameField)
                    throw BuildConfigReaderException( "Unit : A sub-unit config 'UnitRootName' field is missing for '" + info.type + "'.");
                subUnitConfig.unitRootName = subUnitRootNameField.value();

                info.subUnits.emplace_back(subUnitConfig);
            }

            res.unitsInfo.emplace_back(std::move(info));
        }
    }
    catch (const std::exception& e)
    {
        throw BuildConfigReaderException(e.what());
    }
    catch (...)
    {
        throw BuildConfigReaderException("Unkown exception");
    }

    return res;
}
