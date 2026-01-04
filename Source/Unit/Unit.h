#pragma once

#include "Compiler/Compilation.h"
#include <string>
#include <vector>
#include <filesystem>

enum UnitCompilationType
{
    EXECUTABLE,
    STATIC_LIBRARY,
    DYNAMIC_LIBRARY
};

struct UnitModule
{
    std::string name;
};

struct UnitRules
{
    std::string name;
    CppVersion cppVersion;
    UnitCompilationType compilationType;
    std::vector<UnitModule> modules;
};

struct SubUnitInfo
{
    std::filesystem::path dir;
    std::string unitType;
    bool bRecursive;
};

struct UnitInfo
{
    std::string name;
    std::vector<std::filesystem::path> modulesDirs;
    std::filesystem::path buildDir;
    std::filesystem::path targetsDir;
    std::vector<SubUnitInfo> subUnits;
};