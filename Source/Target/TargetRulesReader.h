#pragma once

#include "Target/ITargetRulesReader.h"

namespace sol { class state; }

class TargetRulesReader : public ITargetRulesReader
{
public:
    TargetRulesReader(sol::state& luaState) : lua(luaState) {}

    virtual TargetRules ReadRules(const std::string& targetName, const std::string& targetRulesField) const override;

private:
    sol::state& lua;
};