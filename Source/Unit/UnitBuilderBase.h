#pragma once


#include <exception>
#include <filesystem>

#include "Module/IModuleManager.h"
#include "Target/Target.h"
#include "Unit/Unit.h"
#include "Unit/UnitRulesReader.h"
#include "BuildConfig/BuildConfig.h"


struct ProjectDependancy
{
    std::filesystem::path projectPath;
    std::string unitName;
    std::string unitType;
    std::string buildTarget;
};

struct BuildData
{
    std::filesystem::path unitRoot;
    std::string unitName;
    std::string unitType;
    std::string buildTarget;

    std::vector<ProjectDependancy> dependancyProjects;

    std::filesystem::path configurationFile;
    std::string platform;
};

class UnitBuilderException : public std::exception
{
public:
    UnitBuilderException(const std::string& msg) : message(msg) {}

    const char* what() const noexcept override { return message.c_str(); }

private:
    std::string message;
};

class UnitBuilderBase
{
public:
    UnitBuilderBase(IModuleManager& moduleManager, const std::filesystem::path& buildConfigFile);

    virtual void BuildUnit(const BuildData& buildData);
protected:
    /**
     * @brief Read the unit rules in a lua state
     * @warning The script should be executed before this function call
     *
     * @param luaState The lua state
     * @param buildData The according build data
     */
    virtual void ReadUnitRules(sol::state& luaState, const BuildData& buildData);

    /**
     * @brief Read modules rules in a lua state
     * @warning The script should be executed before this function call
     *
     * @param buildData The according build data
     * @param unitConfig The unit configuration
     * @param unitRulesFile The unit rules file path
     * @param modules The list of modules to read rules for
     */
    virtual void ReadModulesrules(const BuildData& buildData, const UnitConfig& unitConfig, std::filesystem::path unitRulesFile, const std::vector<UnitModule>& modules);

    /**
     * @brief Resolve a macro inside a string.
     *
     * Macro sub-string : ${MacroName}
     *
     * @param str The given string.
     * @param macroName The macro name
     * @param value The replacing value
     * @return The resolved string
     */
    virtual std::string ResolveMacro(const std::string& str, const std::string& macroName, const std::string& value);

protected:
    /**
     * @brief Create a new lua state object from sol library.
     * The build config script is automaticaly executed.
     * 
     * @return The lua state
     */
    sol::state NewBuilderLuaState() const;

    /**
     * @brief Get the suitable unit config for a specific unit type.
     * 
     * @param unitType The unit type
     * @return The corresponding unit config 
     */
    UnitConfig GetUnitConfig(const std::string& unitType) const;

    /**
     * @brief Get the unit rule file path for a build.
     * 
     * @param buildData The build data
     * @return The file path
     */
    std::filesystem::path GetUnitRulesFile(const BuildData& buildData) const;

    UnitRules FetchUnitRules(sol::state& luaState, const BuildData& buildData);


public: // Helper
    /**
     * @brief Get the global Build directory.
     * 
     * @param unitConfig The unit config
     * @param unitRules  The unit rules
     * @return std::filesystem::path The directory
     */
    std::filesystem::path GetBuildDir(std::filesystem::path unitRoot, UnitConfig unitConfig);

    /**
     * @brief Get the build output directory
     * 
     * @param unitConfig 
     * @param unitRules 
     * @param buildData
     * @return std::filesystem::path 
     */
    std::filesystem::path GetBuildOutputDir(const BuildData& buildData);

    /**
     * @brief Get the objects output directory.
     * 
     * @param buildData 
     * @return std::filesystem::path 
     */
    std::filesystem::path GetObjectOutputDir(const BuildData& buildData);

    /**
     * @brief Get the static library output directory path
     * 
     * @param unitRoot 
     * @param unitConfig 
     * @param buildData 
     * @return std::filesystem::path 
     */
    std::filesystem::path GetStaticLibOutputDir(const BuildData& buildData);

    /**
     * @brief Get the binary output directory.
     * 
     * @param buildData 
     * @return std::filesystem::path 
     */
    std::filesystem::path GetBinaryOutputDir(const BuildData& buildData);

protected:
    IModuleManager& moduleManager;

    BuildConfig unitsConfigs;

    std::vector<std::filesystem::path> modulesDirs;

    UnitRules unitRules;
    TargetRules targetRules;

    std::filesystem::path buildConfigFile;
};