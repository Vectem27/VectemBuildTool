
#include "TargetRulesReader.h"
#include "Target/ITargetRulesReader.h"

#include <sol/sol.hpp>

TargetRules TargetRulesReader::ReadRules(const std::string& targetName, const std::string& targetRulesField) const
{
    TargetRules rules;

    rules.name = targetName;

    try
    {
        sol::optional<sol::table> unitRulesField = lua[targetRulesField].get<sol::optional<sol::table>>();
        if (!unitRulesField)
            throw TargetRulesReaderException("Target rules field is missing for : '" + targetName + "'.");

        sol::table unitRules = unitRulesField.value();
    }
    catch (const std::exception& e)
    {
        throw TargetRulesReaderException(e.what());
    }
    catch (...)
    {
        throw TargetRulesReaderException("Unkown exception");
    }

    return rules;
}
