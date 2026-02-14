#pragma once

#include "BuildConfig.h"
#include <stdexcept>

class BuildConfigReaderException : public std::runtime_error
{
public:
    BuildConfigReaderException(std::string msg) : std::runtime_error(msg) {}
};

class IBuildConfigReader
{
public:
    virtual ~IBuildConfigReader() = default;
    virtual BuildConfig ReadBuildConfig(std::filesystem::path unitRoot) const = 0;
};