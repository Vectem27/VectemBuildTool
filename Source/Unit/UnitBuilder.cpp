
#include "UnitBuilder.h"

#include <filesystem>
#include <iostream>
#include <regex>
#include <sol/sol.hpp>

#include "Compiler/ICompiler.h"
#include "Module/IModuleInfoReader.h"
#include "Module/IModuleManager.h"
#include "UnitConfigReader.h"
#include "UnitRulesReader.h"

#include "Module/ModuleInfoReader.h"
#include "Module/ModuleManager.h"

namespace fs = std::filesystem;

void UnitBuilder::BuildUnit(const BuildData& buildData)
{
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::math, sol::lib::string, sol::lib::coroutine, sol::lib::io);

    // Configuration

    lua.safe_script_file(buildData.configurationFile);
    ReadConfiguration(lua, buildData);

    // Target

    //lua.safe_script_file(buildData.buildTargetFile);
    //ReadTarget(lua, buildData);

    // Unit

    lua.safe_script_file(unitRulesFile);
    ReadUnitRules(lua, buildData);

    ReadModulesrules(lua, buildData);

    fs::create_directories(buildOutput);


    // Start compilation

    ModuleManager moduleManager = ModuleManager();

    ICompiler* compiler = compilerFactory.Create();

    IModuleInfoReader* moduleReader = new ModuleInfoReader(lua);

    for (const auto& moduleRules : unitRules.modules)
    {
        // Get module info :

        std::string moduleRootName = unitConfig.moduleRootName;
        moduleRootName = std::regex_replace(moduleRootName, std::regex(R"(\$\{ModuleName\})"), moduleRules.name);

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
        moduleFileName = std::regex_replace(moduleFileName, std::regex(R"(\$\{ModuleName\})"), moduleRules.name);
        fs::path moduleRulesFile = moduleDir / moduleFileName;

        if (!fs::exists(moduleRulesFile) || !fs::is_regular_file(moduleRulesFile))
            throw UnitBuilderException("Missing '" + moduleRulesFile.string() + "' file for : '" + moduleRules.name + "'.");


        ModuleStructureInfo moduleStructure = {
            .rootDir = moduleDir,
            .buildRulesFile = moduleRulesFile,
            .codeDir = "Source"
        };

        moduleManager.AddModule(moduleRules.name, moduleStructure);

        auto moduleInfo = moduleManager.ResolveModuleInfo(moduleRules.name, *moduleReader);

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

    delete moduleReader;
}

void UnitBuilder::ReadConfiguration(sol::state& luaState, const BuildData& buildData)
{
    std::cout << buildData.unitName << std::endl;

    IUnitConfigReader* unitConfigReader = new UnitConfigReader(luaState);
    unitsConfigs = unitConfigReader->ReadUnitsConfig(buildData.unitRoot);
    delete unitConfigReader;

 
    std::string unitFileName = unitsConfigs.UnitFileName;
    unitFileName = std::regex_replace(unitFileName, std::regex(R"(\$\{UnitName\})"), buildData.unitName);
    unitRulesFile = buildData.unitRoot / unitFileName;
}

void UnitBuilder::ReadTarget(sol::state& luaState, const BuildData& buildData)
{
    // TODO: Complete this function
}

void UnitBuilder::ReadUnitRules(sol::state& luaState, const BuildData& buildData) 
{
    IUnitRulesReader* unitRulesReader = new UnitRulesReader(luaState);
    unitRules = unitRulesReader->ReadUnitsRules(buildData.unitName);
    delete unitRulesReader;

    // Config Unit
    for (const auto& unit : unitsConfigs.unitsInfo)
    {
        if (unit.name != unitRules.type)
            continue;

        unitConfig = unit;

        break;
    }

    if (!std::filesystem::exists(unitRulesFile) && std::filesystem::is_regular_file(unitRulesFile))
        throw UnitBuilderException("Unit rules file does not exist for the config unit : '" + unitConfig.name + "'.");

    buildOutput = buildData.unitRoot / unitConfig.buildDir / platform;
}

void UnitBuilder::ReadModulesrules(sol::state& luaState, const BuildData& buildData) 
{
    if (unitConfig.modulesDirs.empty())
        throw UnitBuilderException("Module dir is empty for the config unit : '" + unitConfig.name + "'.");

    for (const auto& moduleDir : unitConfig.modulesDirs)
        modulesDirs.emplace_back(buildData.unitRoot / moduleDir);
}
