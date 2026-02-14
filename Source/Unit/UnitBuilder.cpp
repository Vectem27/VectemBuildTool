
#include "UnitBuilder.h"

#include <filesystem>
#include <iostream>
#include <regex>
#include <sol/sol.hpp>

#include "Compiler/ICompiler.h"
#include "Module/IModuleInfoReader.h"
#include "Module/IModuleManager.h"
#include "BuildConfig/BuildConfigReader.h"
#include "UnitRulesReader.h"

#include "Target/TargetRulesReader.h"

#include "Module/ModuleInfoReader.h"
#include "Module/ModuleManager.h"

namespace fs = std::filesystem;

void UnitBuilder::BuildUnit(const BuildData& buildData)
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
        targetLua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::math, sol::lib::string, sol::lib::coroutine, sol::lib::io);
        targetLua.safe_script_file(buildData.configurationFile.string()); // Build configuration before
        targetLua.safe_script_file(buildTargetFile.string());
        ReadTarget(targetLua, buildData);
    }

    ReadModulesrules(unitLua, buildData);

    fs::create_directories(buildOutput);

    // Start compilation

    ModuleManager moduleManager = ModuleManager();

    ICompiler* compiler = compilerFactory.Create();


    for (const auto& moduleRules : unitRules.modules)
    {
        // Get module info :

        std::string moduleRootName = unitConfig.moduleRootName;
        moduleRootName = ResolveMacro(moduleRootName, "ModuleName", moduleRules.name);

        fs::path moduleDir;
        for (const fs::path& dir : unitConfig.modulesDirs)
        {
            fs::path fullDir = dir / moduleRootName;

            std::cout << "Looking for module '" << moduleRules.name << "' in directory : " << fullDir << std::endl;

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
            .codeDir = "Source"
        };

        moduleManager.AddModule(moduleRules.name, moduleStructure);


        ModuleInfo moduleInfo;
        {
            sol::state moduleLua;
            moduleLua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::math, sol::lib::string, sol::lib::coroutine, sol::lib::io);
            moduleLua.safe_script_file(buildData.configurationFile.string());
            moduleLua.safe_script_file(unitRulesFile.string());
            moduleLua.safe_script_file(moduleRulesFile);
            
            IModuleInfoReader* moduleReader = new ModuleInfoReader(moduleLua);
            moduleInfo = moduleManager.ResolveModuleInfo(
                moduleRules.name, 
                ResolveMacro(unitConfig.moduleClassName, "ModuleName", moduleRules.name), 
                *moduleReader
            );
            delete moduleReader;
        }
        

        std::cout << "Module directory : " << moduleDir << std::endl;

        std::vector<fs::path> cppFiles;
        for (auto& p : fs::recursive_directory_iterator(moduleStructure.rootDir / moduleStructure.codeDir))
        {
            if (!p.is_regular_file())
                continue;

            if (p.path().extension() == ".cpp" || p.path().extension() == ".cxx")
                cppFiles.emplace_back(std::move(p.path()));
        }

        // Compile
        std::cout << "Start module compilation : " << moduleRules.name << std::endl;

        CompileInfo ci = {.outputName = moduleRules.name,
                          .buildOutputPath = buildOutput,
                          .filesToCompile = cppFiles,
                          .includesPaths = {moduleDir},
                          .cppVersion = CppVersion::CPP_20,
                          .optimisation = CompilationOptimisation::OPTIMIZED};

        LibraryCompileInfo lci(ci);

        compiler->CompileLibrary(lci);
    }

    CompileInfo ci = {.outputName = buildData.unitName,
                      .buildOutputPath = buildOutput,
                      .filesToCompile = {},
                      .includesPaths = {},
                      .cppVersion = CppVersion::CPP_20,
                      .optimisation = CompilationOptimisation::OPTIMIZED};

    ExecutableCompileInfo eci(ci);
    eci.staticLibs = {};
    for (const auto& moduleRules : unitRules.modules)
        eci.staticLibs.emplace_back(buildOutput / "lib" / moduleRules.name);

    compiler->CompileExecutable(eci);
    delete compiler;

}

void UnitBuilder::ReadConfiguration(sol::state& luaState, const BuildData& buildData)
{
    std::cout << buildData.unitName << std::endl;

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

void UnitBuilder::ReadUnitRules(sol::state& luaState, const BuildData& buildData) 
{
    {
        IUnitRulesReader* unitRulesReader = new UnitRulesReader(luaState);
        unitRules = unitRulesReader->ReadUnitsRules(
            buildData.unitName, 
            ResolveMacro(unitConfig.unitClassName, "UnitName", buildData.unitName)
        );
        delete unitRulesReader;
    }

    buildOutput = buildData.unitRoot / unitConfig.buildDir / buildData.platform / buildData.buildTarget;

    if(!unitConfig.targetFileName.empty())
    {
        std::string buildTargetFileName = unitConfig.targetFileName;
        buildTargetFileName = std::regex_replace(buildTargetFileName, std::regex(R"(\$\{TargetName\})"), buildData.buildTarget);
        buildTargetFile = unitConfig.targetsDir / buildTargetFileName;
    }
}

void UnitBuilder::ReadTarget(sol::state& luaState, const BuildData& buildData)
{
    ITargetRulesReader* targetRulesReader = new TargetRulesReader(luaState);
    targetRules = targetRulesReader->ReadRules(
        buildData.buildTarget, 
        ResolveMacro(unitConfig.targetClassName, "TargetName", buildData.buildTarget)
    );
    delete targetRulesReader;
}

void UnitBuilder::ReadModulesrules(sol::state& luaState, const BuildData& buildData) 
{
    if (unitConfig.modulesDirs.empty())
        throw UnitBuilderException("Module dir is empty for the config unit : '" + unitConfig.type + "'.");

    for (const auto& moduleDir : unitConfig.modulesDirs)
        modulesDirs.emplace_back(buildData.unitRoot / moduleDir);
}

std::string UnitBuilder::ResolveMacro(const std::string& str, const std::string& macroName, const std::string& value)
{
    return std::regex_replace(str, std::regex("(\\$\\{" + macroName + "\\})"), value);
}
