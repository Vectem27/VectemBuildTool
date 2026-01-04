
#include "UnitBuilder.h"

#include <filesystem>
#include <iostream>
#include <sol/sol.hpp>

#include "Compiler/ICompiler.h"
#include "Module/IModuleInfoReader.h"
#include "UnitConfigReader.h"
#include "UnitRulesReader.h"

#include "Module/ModuleManager.h"
#include "Module/ModuleInfoReader.h"

namespace fs = std::filesystem;

void UnitBuilder::BuildUnit(const BuildData& buildData) const
{
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::math, sol::lib::string, sol::lib::coroutine);

    lua.safe_script_file(buildData.configurationFile);

    IUnitConfigReader* unitConfigReader = new UnitConfigReader(lua);
    std::vector<UnitInfo> unitsConfigs = unitConfigReader->ReadUnitsConfig(buildData.unitRoot);
    delete unitConfigReader;

    std::string platform = "Linux";

    // Configurations check
    for (const auto& conf : unitsConfigs)
    {
        if (conf.modulesDirs.empty())
            throw UnitBuilderException("Module dir is empty for config unit : '" + conf.name + "'.");
        // std::cout << "Unit config [Name : " << conf.name << ", Build dir : " << conf.buildDir <<
        //              ", Target dir : " << conf.targetsDir << ", First module dir : " << conf.modulesDirs.front() <<
        //              "]" << std::endl;
    }

    // Get unit config
    std::filesystem::path unitRulesFile;
    std::vector<std::filesystem::path> modulesDirs;
    std::filesystem::path buildOutput;

    for (const auto& conf : unitsConfigs)
    {
        std::filesystem::path unitRulesFileTest = buildData.unitRoot / (buildData.unitName + "." + conf.name + ".lua");
        if (std::filesystem::exists(unitRulesFileTest) && std::filesystem::is_regular_file(unitRulesFileTest))
        {
            unitRulesFile = unitRulesFileTest;
            buildOutput = buildData.unitRoot / conf.buildDir / platform;
            for (const auto& moduleDir : conf.modulesDirs)
                modulesDirs.emplace_back(std::move(buildData.unitRoot / moduleDir));
            break;
        }
    }


    // TODO : Process target

    fs::create_directories(buildOutput);

    lua.safe_script_file(unitRulesFile);


    IUnitRulesReader* unitRulesReader = new UnitRulesReader(lua);
    UnitRules unitRules = unitRulesReader->ReadUnitsRules(buildData.unitName);
    delete unitRulesReader;

    // Start compilation

    ModuleManager moduleManager = ModuleManager();

    ICompiler* compiler = compilerFactory.Create();

    IModuleInfoReader* moduleReader = new ModuleInfoReader(lua);

    for (const auto& moduleRules : unitRules.modules)
    {
        // Get module info :

        fs::path moduleDir;
        for (const fs::path& dir : modulesDirs)
        {
            fs::path fullDir = dir / moduleRules.name;
            if (fs::exists(fullDir) && fs::is_directory(fullDir))
            {
                moduleDir = fullDir;
                break;
            }
        }

        moduleManager.AddModule(moduleRules.name, moduleDir);
        
        if (moduleDir.empty())
            throw UnitBuilderException("Unable to find module directory for : '" + moduleRules.name + "'.");

        if (!fs::exists(moduleDir / "Module.lua") || ! fs::is_regular_file(moduleDir / "Module.lua"))
            throw UnitBuilderException("Missing 'Module.lua' file for : '" + moduleRules.name + "'.");


        auto moduleInfo = moduleManager.ResolveModuleInfo(moduleRules.name, *moduleReader);

        std::cout << "Module directory : " << moduleDir << std::endl;

        std::vector<fs::path> cppFiles;
        for (auto& p : fs::recursive_directory_iterator(moduleDir)) 
        {
            if (!p.is_regular_file())
                continue;

            if (p.path().extension() == ".cpp" || p.path().extension() == ".cxx")
                cppFiles.emplace_back(std::move(p.path()));
        }


        // Compile
        std::cout << "Start module compilation : " << moduleRules.name << std::endl;
        
        CompileInfo ci = {
            .outputName = moduleRules.name,
            .buildOutputPath = buildOutput,
            .filesToCompile = cppFiles,
            .includesPaths = {moduleDir},
            .cppVersion = CppVersion::CPP_20,
            .optimisation = CompilationOptimisation::OPTIMIZED
        };

        LibraryCompileInfo lci(ci);

        compiler->CompileLibrary(lci);
    }

    CompileInfo ci = {
        .outputName = buildData.unitName,
        .buildOutputPath = buildOutput,
        .filesToCompile = {},
        .includesPaths = {},
        .cppVersion = CppVersion::CPP_20,
        .optimisation = CompilationOptimisation::OPTIMIZED
    };

    ExecutableCompileInfo eci(ci);
    eci.staticLibs = {};
    for (const auto& moduleRules : unitRules.modules)
        eci.staticLibs.emplace_back(buildOutput / "lib" / moduleRules.name);

    compiler->CompileExecutable(eci);
    delete compiler;

    delete moduleReader;
}
