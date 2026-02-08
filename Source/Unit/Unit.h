#pragma once

#include "Compiler/Compilation.h"
#include <string>
#include <vector>
#include <filesystem>


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
    std::string type;
    CppVersion cppVersion;
    UnitCompilationType compilationType;
    std::vector<UnitModule> modules;
};

/**
 * @brief Structure representing subunit info for unit config info
 */
struct SubUnitInfo
{
    std::filesystem::path dir;
    std::string unitType;
    std::string unitRootName;
    bool bRecursive;
};

/**
 * @brief Structure representing a unit config infos
 */
struct UnitInfo
{
    std::string name;
    
    std::vector<std::filesystem::path> modulesDirs;
    std::string moduleRootName;
    std::string moduleFileName;
    std::string moduleClassName;

    std::filesystem::path buildDir;
    std::filesystem::path targetsDir;
    std::vector<SubUnitInfo> subUnits;
};

struct UnitsConfig
{
    std::string UnitFileName;
    std::vector<UnitInfo> unitsInfo;
};