#pragma once

#include "Compiler/Compilation.h"
#include <string>
#include <vector>

/**
 * @brief Enumeration descripting a unit compilation type
 */
enum UnitCompilationType
{
    EXECUTABLE,
    STATIC_LIBRARY,
    DYNAMIC_LIBRARY
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
    CppVersion cppVersion;
    UnitCompilationType compilationType;
    std::vector<UnitModule> modules;
};