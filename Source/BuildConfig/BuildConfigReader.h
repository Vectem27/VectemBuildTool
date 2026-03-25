#pragma once

#include "BuildConfig/IBuildConfigReader.h"

namespace sol { class state; }



class BuildConfigReader : public IBuildConfigReader
{
public:
    BuildConfigReader(sol::state& luaState) : lua(luaState) {}

    virtual BuildConfig ReadBuildConfig() const override;

private:
    sol::state& lua;
};