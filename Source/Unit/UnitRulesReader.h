#pragma once

#include "IUnitRulesReader.h"

namespace sol { class state; }

class UnitRulesReader : public IUnitRulesReader
{
public:
    UnitRulesReader(sol::state& luaState) : lua(luaState) {}

    virtual UnitRules ReadUnitsRules(const std::string& unitName, const std::string& unitRulesField) const override;

private:
    sol::state& lua;
};