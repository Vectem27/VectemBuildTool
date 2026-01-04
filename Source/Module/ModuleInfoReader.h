#pragma once

#include "IModuleInfoReader.h"

namespace sol { class state; }

class ModuleInfoReader : public IModuleInfoReader
{
public:
    ModuleInfoReader(sol::state& luaState) : lua(luaState) {}

    virtual ModuleInfo ReadInfo(std::string filePath) const override;
    
private:
    sol::state& lua;
};