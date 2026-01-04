#pragma once

#include "Unit.h"
#include <stdexcept>

class UnitRulesReaderException : public std::runtime_error
{
public:
    UnitRulesReaderException(std::string msg) : std::runtime_error(msg) {}
};

class IUnitRulesReader
{
public:
    virtual ~IUnitRulesReader() = default;
    virtual UnitRules ReadUnitsRules(std::string unitName) const = 0;
};