#pragma once

#include <exception>
#include <filesystem>

#include "../Compiler/ICompiler.h"
#include "Target/Target.h"
#include "Unit/Unit.h"
#include "BuildConfig/BuildConfig.h"
#include "Unit/UnitRulesReader.h"

class ITargetRulesReader;

struct BuildData
{
    std::filesystem::path unitRoot;
    std::string unitName;
    std::string unitType;
    std::string buildTarget;
    
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


class UnitBuilder
{
public:
    UnitBuilder(const ICompilerFactory& compilerFactory) : compilerFactory(compilerFactory) {}

    void BuildUnit(const BuildData& buildData);

protected:
    /** 
     * @brief Read configutation data in a lua state
     * @warning The script should be executed before this function call
     * 
     * @param luaState The lua state
     * @param buildData The according build data
     */
    void ReadConfiguration(sol::state& luaState, const BuildData& buildData);   
    
    /** 
     * @brief Read the target data in a lua state
     * @warning The script should be executed before this function call
     * 
     * @param luaState The lua state
     * @param buildData The according build data
     */
    void ReadTarget(sol::state& luaState, const BuildData& buildData);  

    /** 
     * @brief Read the unit rules in a lua state
     * @warning The script should be executed before this function call
     * 
     * @param luaState The lua state
     * @param buildData The according build data
     */
    void ReadUnitRules(sol::state& luaState, const BuildData& buildData); 

    /** 
     * @brief Read modules rules in a lua state
     * @warning The script should be executed before this function call
     * 
     * @param luaState The lua state
     * @param buildData The according build data
     */
    void ReadModulesrules(sol::state& luaState, const BuildData& buildData); 

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
    std::string ResolveMacro(const std::string& str, const std::string& macroName, const std::string& value);

private:
    const ICompilerFactory& compilerFactory;

    BuildConfig unitsConfigs;
    UnitConfig unitConfig;

    std::filesystem::path unitRulesFile;
    std::filesystem::path buildTargetFile;
    std::vector<std::filesystem::path> modulesDirs;
    std::filesystem::path buildOutput;


    UnitRules unitRules;
    TargetRules targetRules;
};