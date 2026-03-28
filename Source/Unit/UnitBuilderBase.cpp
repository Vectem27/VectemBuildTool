#include "UnitBuilderBase.h"

#include <regex>

#include <sol/sol.hpp>

#include "BuildConfig/BuildConfig.h"
#include "BuildConfig/BuildConfigReader.h"
#include "Core/Logger.hpp"
#include "Target/TargetRulesReader.h"

#include "Module/ModuleInfoReader.h"

namespace fs = std::filesystem;


UnitBuilderBase::UnitBuilderBase(IModuleManager& moduleManager, const fs::path& buildConfigFile)
    : moduleManager(moduleManager), buildConfigFile(buildConfigFile)
{
    Logger::Log(LogLevel::Debug, "Reading configuration file: %s", buildConfigFile.c_str());

    sol::state luaState = NewBuilderLuaState();

    auto buildConfigReader = std::make_unique<BuildConfigReader>(luaState);
    unitsConfigs = buildConfigReader->ReadBuildConfig();
}

void UnitBuilderBase::BuildUnit(const BuildData& buildData) 
{
    sol::state unitLua = NewBuilderLuaState();

    auto unitRulesFile = GetUnitRulesFile(buildData);

    auto unitConfig = GetUnitConfig(buildData.unitType);

    // Unit
    unitLua.safe_script_file(unitRulesFile.string());
    ReadUnitRules(unitLua, buildData);

    // Target
    {
        if (unitConfig.targetFileName.empty())
            throw UnitBuilderException("Unable to build '" + buildData.unitName +"' : Target filename is empty.");
       
        fs::path buildTargetFile;
         std::string buildTargetFileName = unitConfig.targetFileName;
        buildTargetFileName = std::regex_replace(buildTargetFileName, std::regex(R"(\$\{TargetName\})"), buildData.buildTarget);
        buildTargetFile = buildData.unitRoot / unitConfig.targetsDir / buildTargetFileName;
       

        sol::state targetLua = NewBuilderLuaState();
        targetLua.safe_script_file(buildTargetFile.string());

        auto targetRulesReader = std::make_unique<TargetRulesReader>(targetLua);
        targetRules = targetRulesReader->ReadRules(
            buildData.buildTarget,
            ResolveMacro(unitConfig.targetClassName, "TargetName", buildData.buildTarget));
    }

    ReadModulesrules(buildData, unitConfig, unitRulesFile, unitRules.modules);
}

void UnitBuilderBase::ReadUnitRules(sol::state& luaState, const BuildData& buildData)
{
    unitRules = FetchUnitRules(luaState, buildData);

}

void UnitBuilderBase::ReadModulesrules(const BuildData& buildData, const UnitConfig& unitConfig, std::filesystem::path unitRulesFile, const std::vector<UnitModule>& modules)
{
    if (unitConfig.modulesDirs.empty())
        throw UnitBuilderException("Module dir is empty for the config unit : '" + unitConfig.type + "'.");

    for (const auto& moduleDir : unitConfig.modulesDirs)
        modulesDirs.emplace_back(buildData.unitRoot / moduleDir);

    
    for (const auto& moduleRules : modules)
    {
        // Get module info :

        std::string moduleRootName = unitConfig.moduleRootName;
        moduleRootName = ResolveMacro(moduleRootName, "ModuleName", moduleRules.name);

        fs::path moduleDir;
        for (const fs::path& dir : unitConfig.modulesDirs)
        {
            fs::path fullDir = buildData.unitRoot / dir / moduleRootName;

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

sol::state UnitBuilderBase::NewBuilderLuaState() const
{
    sol::state unitLua;
    unitLua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::math, sol::lib::string, sol::lib::coroutine, sol::lib::io);
    unitLua.safe_script_file(buildConfigFile.string());
    return unitLua;
}

UnitConfig UnitBuilderBase::GetUnitConfig(const std::string& unitType) const
{ 
    UnitConfig unitConfig;

    for (const auto& unit : unitsConfigs.unitsInfo)
    {
        if (unit.type != unitType)
            continue;

        unitConfig = unit;
        break;
    }

    if (unitConfig.type.empty())
        throw UnitBuilderException("No config set for unit type : '" + unitType + "'.");

    return unitConfig;
}

fs::path UnitBuilderBase::GetUnitRulesFile(const BuildData& buildData) const
{
    auto unitConfig = GetUnitConfig(buildData.unitType);
    std::string unitFileName = unitConfig.unitFileName;
    unitFileName = std::regex_replace(unitFileName, std::regex(R"(\$\{UnitName\})"), buildData.unitName);
    auto unitRulesFile = buildData.unitRoot / unitFileName;

    if (!std::filesystem::exists(unitRulesFile) || !std::filesystem::is_regular_file(unitRulesFile))
        throw UnitBuilderException("Unit rules file does not exist for the config unit : '" + unitConfig.type + "'.");

    return unitRulesFile;
}

UnitRules UnitBuilderBase::FetchUnitRules(sol::state& luaState, const BuildData& buildData) 
{ 
    auto unitConfig = GetUnitConfig(buildData.unitType);
    std::unique_ptr<IUnitRulesReader> unitRulesReader = std::make_unique<UnitRulesReader>(luaState);
    return unitRulesReader->ReadUnitsRules(buildData.unitName, ResolveMacro(unitConfig.unitClassName, "UnitName", buildData.unitName));
}

std::filesystem::path UnitBuilderBase::GetBuildDir(std::filesystem::path unitRoot, UnitConfig unitConfig) 
{
    return unitRoot / unitConfig.buildDir;
}

std::filesystem::path UnitBuilderBase::GetBuildOutputDir(UnitConfig unitConfig, const BuildData& buildData) 
{
    return GetBuildDir(buildData.unitRoot, unitConfig) / buildData.platform / buildData.buildTarget;
}
std::filesystem::path UnitBuilderBase::GetStaticLibOutputDir(UnitConfig unitConfig, const BuildData& buildConfig) 
{
    return GetBuildOutputDir(unitConfig, buildConfig) / "lib";
}