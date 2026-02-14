#pragma once

#include <string>
#include <vector>
#include <filesystem>

/**
 * @brief Structure representing subunit info for unit config info
 */
struct SubUnitConfig
{
    std::filesystem::path dir;
    std::string unitType;
    std::string unitRootName;
    bool bRecursive;
};

/**
 * @brief Structure representing a unit config infos
 */
struct UnitConfig
{
    std::string type;

    std::string unitFileName;
    std::string unitClassName;

    std::vector<std::filesystem::path> modulesDirs;
    std::string moduleRootName;
    std::string moduleFileName;
    std::string moduleClassName;

    std::filesystem::path targetsDir;
    std::string targetFileName;
    std::string targetClassName;

    std::filesystem::path buildDir;

    std::vector<SubUnitConfig> subUnits;
};

struct BuildConfig
{
    std::vector<UnitConfig> unitsInfo;
};