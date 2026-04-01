#pragma once

#include "Compiler/Compilation.h"
#include <string>
#include <vector>

enum class UnitCompilationType
{
    Executable,
    StaticLibrary,
    DynamicLibrary
};

/**
 * @brief Structure holding modules info
 */
struct UnitModule
{
    std::string name;
};

/**
 * @brief Structure for unit compilation and global data
 */
struct UnitRules
{
    std::string name;
    std::vector<UnitModule> modules;
    UnitCompilationType compilationType;
};