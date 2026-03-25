#include "UnitBuilderBase.h"

#include <regex>

#include <sol/sol.hpp>

#include "BuildConfig/BuildConfigReader.h"
#include "Core/Logger.hpp"
#include "Target/TargetRulesReader.h"

#include "Module/ModuleInfoReader.h"

namespace fs = std::filesystem;

void UnitBuilderBase::BuildUnit(const BuildData& buildData) 
{
    sol::state unitLua;
    unitLua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::math, sol::lib::string, sol::lib::coroutine, sol::lib::io);

    // Configuration
    unitLua.safe_script_file(buildData.configurationFile.string());
    ReadConfiguration(unitLua, buildData);

    // Unit
    unitLua.safe_script_file(unitRulesFile.string());
    ReadUnitRules(unitLua, buildData);

    // Target
    {
        sol::state targetLua;
        targetLua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::math, sol::lib::string, sol::lib::coroutine,
                                 sol::lib::io);
        targetLua.safe_script_file(buildData.configurationFile.string()); // Build configuration before
        targetLua.safe_script_file(buildTargetFile.string());
        ReadTarget(targetLua, buildData);
    }

    ReadModulesrules(buildData);
}


void UnitBuilderBase::ReadConfiguration(sol::state& luaState, const BuildData& buildData)
{
    Logger::Log(LogLevel::Debug, "Reading configuration for unit: %s", buildData.unitName.c_str());

    IBuildConfigReader* buildConfigReader = new BuildConfigReader(luaState);
    unitsConfigs = buildConfigReader->ReadBuildConfig(buildData.unitRoot);
    delete buildConfigReader;

    // Config Unit
    for (const auto& unit : unitsConfigs.unitsInfo)
    {
        if (unit.type != buildData.unitType)
            continue;

        unitConfig = unit;
        break;
    }

    if (unitConfig.type.empty())
        throw UnitBuilderException("No config set for unit type : '" + unitConfig.type + "'.");

    // Unit rules file name

    std::string unitFileName = unitConfig.unitFileName;
    unitFileName = std::regex_replace(unitFileName, std::regex(R"(\$\{UnitName\})"), buildData.unitName);
    unitRulesFile = buildData.unitRoot / unitFileName;

    if (!std::filesystem::exists(unitRulesFile) || !std::filesystem::is_regular_file(unitRulesFile))
        throw UnitBuilderException("Unit rules file does not exist for the config unit : '" + unitConfig.type + "'.");
}

void UnitBuilderBase::ReadUnitRules(sol::state& luaState, const BuildData& buildData)
{
    {
        IUnitRulesReader* unitRulesReader = new UnitRulesReader(luaState);
        unitRules = unitRulesReader->ReadUnitsRules(buildData.unitName,
                                                    ResolveMacro(unitConfig.unitClassName, "UnitName", buildData.unitName));
        delete unitRulesReader;
    }

    buildOutput = buildData.unitRoot / unitConfig.buildDir / buildData.platform / buildData.buildTarget;

    if (!unitConfig.targetFileName.empty())
    {
        std::string buildTargetFileName = unitConfig.targetFileName;
        buildTargetFileName = std::regex_replace(buildTargetFileName, std::regex(R"(\$\{TargetName\})"), buildData.buildTarget);
        buildTargetFile = unitConfig.targetsDir / buildTargetFileName;
    }
}

void UnitBuilderBase::ReadTarget(sol::state& luaState, const BuildData& buildData)
{
    ITargetRulesReader* targetRulesReader = new TargetRulesReader(luaState);
    targetRules = targetRulesReader->ReadRules(buildData.buildTarget,
                                               ResolveMacro(unitConfig.targetClassName, "TargetName", buildData.buildTarget));
    delete targetRulesReader;
}

void UnitBuilderBase::ReadModulesrules(const BuildData& buildData)
{
    if (unitConfig.modulesDirs.empty())
        throw UnitBuilderException("Module dir is empty for the config unit : '" + unitConfig.type + "'.");

    for (const auto& moduleDir : unitConfig.modulesDirs)
        modulesDirs.emplace_back(buildData.unitRoot / moduleDir);

    
    for (const auto& moduleRules : unitRules.modules)
    {
        // Get module info :

        std::string moduleRootName = unitConfig.moduleRootName;
        moduleRootName = ResolveMacro(moduleRootName, "ModuleName", moduleRules.name);

        fs::path moduleDir;
        for (const fs::path& dir : unitConfig.modulesDirs)
        {
            fs::path fullDir = dir / moduleRootName;

            Logger::Log(LogLevel::Debug, "Looking for module '%s' in directory: %s", moduleRules.name.c_str(), fullDir.string().c_str());

            if (fs::exists(fullDir) && fs::is_directory(fullDir))
            {
                moduleDir = fullDir;
                break;
            }
        }

        if (moduleDir.empty())
            throw UnitBuilderException("Unable to find module directory : '" + moduleDir.string() + "' for module '" +
                                       moduleRules.name + "'.");
        std::string moduleFileName = unitConfig.moduleFileName;
        moduleFileName = ResolveMacro(moduleFileName, "ModuleName", moduleRules.name);
        fs::path moduleRulesFile = moduleDir / moduleFileName;

        if (!fs::exists(moduleRulesFile) || !fs::is_regular_file(moduleRulesFile))
            throw UnitBuilderException("Missing '" + moduleRulesFile.string() + "' file for : '" + moduleRules.name + "'.");

        ModuleStructureInfo moduleStructure = {
            .rootDir = moduleDir,
            .buildRulesFile = moduleRulesFile,
        };

        sol::state moduleLua;
        moduleLua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::math, sol::lib::string, sol::lib::coroutine,
                                 sol::lib::io);
        moduleLua.safe_script_file(buildData.configurationFile.string());
        moduleLua.safe_script_file(unitRulesFile.string());
        moduleLua.safe_script_file(moduleRulesFile);

        std::unique_ptr<IModuleInfoReader> moduleReader = std::make_unique<ModuleInfoReader>(moduleLua);

        moduleManager.AddModule(moduleRules.name, moduleStructure,
                                 ResolveMacro(unitConfig.moduleClassName, "ModuleName", moduleRules.name), *moduleReader);
    }
}

std::string UnitBuilderBase::ResolveMacro(const std::string& str, const std::string& macroName, const std::string& value)
{
    return std::regex_replace(str, std::regex("(\\$\\{" + macroName + "\\})"), value);
}

