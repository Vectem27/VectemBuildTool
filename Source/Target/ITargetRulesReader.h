#pragma once

#include <exception>

#include "Target.h"

class TargetRulesReaderException : public std::exception
{
public:
    TargetRulesReaderException(std::string msg) : msg(msg) {}
    virtual const char* what() const noexcept override { return msg.c_str(); }
private:
    std::string msg;
};

class ITargetRulesReader
{
public:
    virtual ~ITargetRulesReader() = default;
    virtual TargetRules ReadRules(const std::string& targetName, const std::string& targetRulesField) const = 0;
};
