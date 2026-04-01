
#include "UnitRulesReader.h"
#include "Unit/Unit.h"

#include <sol/optional_implementation.hpp>
#include <sol/sol.hpp>

UnitRules UnitRulesReader::ReadUnitsRules(const std::string& unitName, const std::string& unitRulesFieldName) const 
{
    UnitRules res;

    try
    {
        // Module table
        sol::optional<sol::table> unitRulesField = lua[unitRulesFieldName].get<sol::optional<sol::table>>();
        if (!unitRulesField)
            throw UnitRulesReaderException("Unit rules is missing for : '" + unitName + "'.");

        sol::table unitRules = unitRulesField.value();

        res.name = unitName;


        // Compilation type
        sol::optional<std::string> compilationType = unitRules["CompilationType"].get<sol::optional<std::string>>();
        if (!compilationType)
            throw UnitRulesReaderException("Unit compilation type is missing for : '" + unitName + "'.");

        if (compilationType == "Executable")
            res.compilationType = UnitCompilationType::Executable;
        else if (compilationType == "StaticLibrary")
            res.compilationType = UnitCompilationType::StaticLibrary;
        else if (compilationType == "DynamicLibrary")
            res.compilationType = UnitCompilationType::DynamicLibrary;
        else
            throw UnitRulesReaderException("Unknown compilation type for unit '" + unitName + "': " + compilationType.value());


        sol::table modules = unitRules["Modules"];
        if (!modules.valid())
            throw UnitRulesReaderException("Unit modules field is missing for : '" + unitName + "'.");

        for (const auto& pair : modules)
        {
            UnitModule module;
            
            sol::object key = pair.first;
            sol::object value = pair.second;

            // Name
            if (!key.is<std::string>())
                throw UnitRulesReaderException("Unit module rules key is not a string.");

            module.name = key.as<std::string>();
            // infos

            if (!value.is<sol::table>())
                throw UnitRulesReaderException("Unit module rules field is not a table for '" + 
                                                    module.name + "'.");

            auto moduleRulesTable = value.as<sol::table>();

            // Process module rules here if needed
        
            res.modules.emplace_back(std::move(module));
        }

    }
    catch (const std::exception& e)
    {
        throw UnitRulesReaderException(e.what());
    }
    catch (...)
    {
        throw UnitRulesReaderException("Unkown exception");
    }

    return res;
}