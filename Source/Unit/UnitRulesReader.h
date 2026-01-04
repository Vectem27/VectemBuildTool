#pragma once

#include "IUnitRulesReader.h"

namespace sol { class state; }

class UnitRulesReader : public IUnitRulesReader
{
public:
    UnitRulesReader(sol::state& luaState) : lua(luaState) {}

    virtual UnitRules ReadUnitsRules(std::string unitName) const override;

private:
    sol::state& lua;
};