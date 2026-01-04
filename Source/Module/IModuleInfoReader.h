#pragma once

#include <exception>

#include "Module.h"

class ModuleInfoReaderException : public std::exception
{
public:
    ModuleInfoReaderException(std::string msg) : msg(msg) {}
    virtual const char* what() const noexcept override { return msg.c_str(); }
private:
    std::string msg;
};

class IModuleInfoReader
{
public:
    virtual ~IModuleInfoReader() = default;

    virtual ModuleInfo ReadInfo(std::string filePath) const = 0;
};
