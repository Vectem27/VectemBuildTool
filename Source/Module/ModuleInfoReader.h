#pragma once

#include "IModuleInfoReader.h"

namespace sol { class state; }

class ModuleInfoReader : public IModuleInfoReader
{
public:
    ModuleInfoReader(sol::state& luaState) : lua(luaState) {}

    virtual ModuleInfo ReadInfo(const std::string& moduleName, const std::string& moduleRulesField) const override;
    
private:
    sol::state& lua;
};