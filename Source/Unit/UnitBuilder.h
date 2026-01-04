#pragma once

#include <exception>
#include <filesystem>

#include "../Compiler/ICompiler.h"

struct BuildData
{
    std::string unitName;
    std::filesystem::path unitRoot;
    std::filesystem::path configurationFile;
    std::filesystem::path buildTargetFile;
};

class UnitBuilderException : public std::exception 
{
public:
    UnitBuilderException(const std::string& msg) : message(msg) {}

    const char* what() const noexcept override { return message.c_str(); }

private:
    std::string message;
};


class UnitBuilder
{
public:
    UnitBuilder(const ICompilerFactory& compilerFactory) : compilerFactory(compilerFactory) {}

    void BuildUnit(const BuildData& buildData) const;

private:
    const ICompilerFactory& compilerFactory;
};