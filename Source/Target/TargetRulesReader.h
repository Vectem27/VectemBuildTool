#pragma once

#include "Target/ITargetRulesReader.h"

namespace sol { class state; }

class TargetRulesReader : public ITargetRulesReader
{
public:
    TargetRulesReader(sol::state& luaState) : lua(luaState) {}

    virtual TargetRules ReadRules(std::string targetName, std::string targetFieldName) const override;

private:
    sol::state& lua;
};