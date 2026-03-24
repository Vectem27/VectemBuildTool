#pragma once

#include "Target/ITargetRulesReader.h"
#include "Target/Target.h"
#include "Compiler/Compilation.h"

namespace sol { class state; }

class TargetRulesReader : public ITargetRulesReader
{
public:
    TargetRulesReader(sol::state& luaState) : lua(luaState) {}

    virtual TargetRules ReadRules(const std::string& targetName, const std::string& targetRulesField) const override;

private:
    sol::state& lua;
    
    CVersion StringToCVersion(const std::string& value) const;
    CppVersion StringToCppVersion(const std::string& value) const;
    CompilationOptimisation StringToOptimisationType(const std::string& value) const;
    FloatingPointModel StringToFloatingPointType(const std::string& value) const;
};