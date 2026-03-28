
#include "UnitBuilder.h"

#include <filesystem>

#include <sol/sol.hpp>
#include <vector>

#include "Core/Logger.hpp"
#include "Compiler/ICompiler.h"
#include "Module/IModuleDependencySorter.h"
#include "Module/IModuleManager.h"
#include "Module/Module.h"
#include "Unit/UnitBuilderBase.h"

namespace fs = std::filesystem;

UnitBuilder::UnitBuilder(IModuleManager& moduleManager, const fs::path& buildConfigFile, const ICompilerFactory& compilerFactory, const IModuleDependencySorter& moduleDepSorter,
                         const IModuleIncludeSolver& moduleIncSolver)
    : UnitBuilderBase(moduleManager, buildConfigFile), compilerFactory(compilerFactory), moduleDepSorter(moduleDepSorter), moduleIncSolver(moduleIncSolver)
{

}

void UnitBuilder::BuildUnit(const BuildData& buildData)
{
    // TODO: Compile sub-units
    
    UnitBuilderBase::BuildUnit(buildData);
    
    // TODO: Add sub folder to copy (For script for example)
    // TODO: Add native external libraries support

    std::vector<DependancyProcessingResult> dependanciesData;
    dependanciesData.reserve(buildData.dependancyProjects.size());
    for (const auto& dependancy : buildData.dependancyProjects)
    {
        Logger::Log(LogLevel::Info, "Fetch dependancy project: %s", dependancy.unitName.c_str());
        auto res = ProcessDependancyProject(dependancy, buildData);

        dependanciesData.emplace_back(std::move(res));
    }

    auto buildOutput = GetBuildOutputDir(buildData);

    fs::create_directories(buildOutput);

    // Start compilation

    ICompiler* compiler = compilerFactory.Create();

    for (const auto& moduleRules : unitRules.modules)
    {
        ModuleInfo moduleInfo = moduleManager.ResolveModuleInfo(moduleRules.name);

        IncludesToAdd includes = moduleIncSolver.Resolve(moduleInfo.name, moduleManager);

        std::vector<fs::path> cppFiles;
        for (auto& p : fs::recursive_directory_iterator(moduleInfo.directory / moduleInfo.codeDir))
        {
            if (!p.is_regular_file())
                continue;

            if (p.path().extension() == ".cpp" || p.path().extension() == ".cxx")
                cppFiles.emplace_back(std::move(p.path()));
        }

        // Compile
        Logger::Log(LogLevel::Info, "Start module compilation: %s", moduleRules.name.c_str());

        CompileInfo ci = {.outputName = moduleRules.name,
                          .buildOutputPath = buildOutput,
                          .filesToCompile = cppFiles,
                          .includesPaths = includes,
                          .bAddDebugInfo = targetRules.bAddDebugInfo,
                          .cVersion = targetRules.cVersion,
                          .cppVersion = targetRules.cppVersion,
                          .supportedPlatforms = targetRules.supportedPlatforms,
                          .optimisation = targetRules.optimisationType,
                          .floatingPointModel = targetRules.floatingPointType,
                          };
        LibraryCompileInfo lci{ci};
        lci.linkType = LinkType::STATIC;
        compiler->CompileLibrary(lci);
    }

    if (unitRules.compilationType == UnitCompilationType::Executable)
    {
        std::vector<std::string> moduleList = moduleManager.GetModuleNames();

        SortedModulesGroups sortedModules = moduleDepSorter.Sort(moduleList, moduleManager);

        CompileInfo ci = {.outputName = buildData.unitName,
                        .buildOutputPath = buildOutput,
                        .filesToCompile = {},
                        .includesPaths = {},
                        .bAddDebugInfo = targetRules.bAddDebugInfo,
                        .cVersion = targetRules.cVersion,
                        .cppVersion = targetRules.cppVersion,
                        .supportedPlatforms = targetRules.supportedPlatforms,
                        .optimisation = targetRules.optimisationType,
                        .floatingPointModel = targetRules.floatingPointType};

        for (const auto& group : sortedModules)
        {
            std::vector<std::filesystem::path> groupPaths;
            for (const auto& module : group)
                groupPaths.emplace_back(buildOutput / "lib" / module);
            ci.staticLibsToLink.emplace_back(groupPaths);
        }

        ci.staticLibPaths.emplace_back(buildOutput / "lib");

        for (const auto& depData : dependanciesData)
        ci.staticLibPaths.emplace_back(depData.libDir);
            


        // TODO: Add dependancies static libs to link 

        ExecutableCompileInfo eci(ci);
        //eci.staticLibs = {};
        //for (const auto& moduleRules : unitRules.modules)
        //    eci.staticLibs.emplace_back(buildOutput / "lib" / moduleRules.name);


        compiler->CompileExecutable(eci);
    }
    
    delete compiler;
}

UnitBuilder::DependancyProcessingResult UnitBuilder::ProcessDependancyProject(const ProjectDependancy& dependancy, const BuildData& buildData) 
{
    DependancyProcessingResult res;

    // Prepare a Lua state for the dependency and read its build config + unit rules
    sol::state luaState = NewBuilderLuaState();

    BuildData dependancyBuildData = {
        .unitRoot = dependancy.projectPath,
        .unitName = dependancy.unitName,
        .unitType = dependancy.unitType,
        .buildTarget = buildData.buildTarget,
        .dependancyProjects = {},
        .configurationFile = buildData.configurationFile,
        .platform = buildData.platform
    };


    UnitConfig unitConfig = GetUnitConfig(dependancyBuildData.unitType);

    res.libDir = GetStaticLibOutputDir(dependancyBuildData);

    std::filesystem::path unitRulesFile = GetUnitRulesFile(dependancyBuildData);

    luaState.safe_script_file(unitRulesFile.string());

    auto unitRules = FetchUnitRules(luaState, dependancyBuildData);

    ReadModulesrules(dependancyBuildData, unitConfig, unitRulesFile, unitRules.modules);
    return res;
}
