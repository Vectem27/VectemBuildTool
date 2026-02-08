#pragma once

#include "IUnitConfigReader.h"

namespace sol { class state; }



class UnitConfigReader : public IUnitConfigReader
{
public:
    UnitConfigReader(sol::state& luaState) : lua(luaState) {}

    virtual UnitsConfig ReadUnitsConfig(std::filesystem::path unitRoot) const override;

private:
    sol::state& lua;
};