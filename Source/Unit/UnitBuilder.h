#pragma once

#include <exception>
#include <filesystem>

#include "../Compiler/ICompiler.h"
#include "Unit/Unit.h"
#include "Unit/UnitRulesReader.h"

struct BuildData
{
    std::string unitName;
    std::filesystem::path unitRoot;
    std::filesystem::path configurationFile;
    std::string buildTarget;
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

private:
    const ICompilerFactory& compilerFactory;

    UnitsConfig unitsConfigs;
    UnitInfo unitConfig;

    std::filesystem::path unitRulesFile;
    std::filesystem::path buildTargetFile;
    std::vector<std::filesystem::path> modulesDirs;
    std::filesystem::path buildOutput;

    std::string platform = "Linux";

    UnitRules unitRules;
};