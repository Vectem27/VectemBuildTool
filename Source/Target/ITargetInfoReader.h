#pragma once

#include <exception>

#include "Target.h"

class TargetInfoReaderException : public std::exception
{
public:
    TargetInfoReaderException(std::string msg) : msg(msg) {}
    virtual const char* what() const noexcept override { return msg.c_str(); }
private:
    std::string msg;
};

class ITargetInfoReader
{
public:
    virtual ~ITargetInfoReader() = default;
    virtual TargetInfo ReadInfo(std::string filePath) const = 0;
};
