
#include "TargetRulesReader.h"
#include "Target/ITargetRulesReader.h"

#include <sol/sol.hpp>

TargetRules TargetRulesReader::ReadRules(std::string targetName, std::string targetFieldName) const
{
    TargetRules rules;

    rules.name = targetName;

    try
    {
        sol::table unitRules = lua[targetFieldName];
        if (!unitRules.valid())
            throw TargetRulesReaderException("Target rules field is missing for : '" + targetName + "'.");
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
