
#include "UnitRulesReader.h"
#include "Compiler/Compilation.h"
#include "Unit/Unit.h"

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

    res.compilationType = UnitCompilationType::EXECUTABLE;
    res.cppVersion = CppVersion::CPP_20;

    return res;
}