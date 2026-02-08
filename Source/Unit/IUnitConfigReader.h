#pragma once

#include "Unit.h"
#include <stdexcept>

class UnitConfigReaderException : public std::runtime_error
{
public:
    UnitConfigReaderException(std::string msg) : std::runtime_error(msg) {}
};

class IUnitConfigReader
{
public:
    virtual ~IUnitConfigReader() = default;
    virtual UnitsConfig ReadUnitsConfig(std::filesystem::path unitRoot) const = 0;
};