#pragma once

#include "BuildConfig/IBuildConfigReader.h"

namespace sol { class state; }



class BuildConfigReader : public IBuildConfigReader
{
public:
    BuildConfigReader(sol::state& luaState) : lua(luaState) {}

    virtual BuildConfig ReadBuildConfig(std::filesystem::path unitRoot) const override;

private:
    sol::state& lua;
};