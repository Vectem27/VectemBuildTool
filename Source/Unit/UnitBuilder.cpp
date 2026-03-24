
#include "UnitBuilder.h"

#include <filesystem>
#include <iostream>

#include <sol/sol.hpp>

#include "Compiler/ICompiler.h"
#include "Module/IModuleDependencySorter.h"
#include "Module/IModuleManager.h"
#include "Module/Module.h"
#include "Unit/UnitBuilderBase.h"

namespace fs = std::filesystem;

UnitBuilder::UnitBuilder(IModuleManager& moduleManager, const ICompilerFactory& compilerFactory, const IModuleDependencySorter& moduleDepSorter,
                         const IModuleIncludeSolver& moduleIncSolver)
    : UnitBuilderBase(moduleManager), compilerFactory(compilerFactory), moduleDepSorter(moduleDepSorter), moduleIncSolver(moduleIncSolver)
{

}

void UnitBuilder::BuildUnit(const BuildData& buildData)
{
    UnitBuilderBase::BuildUnit(buildData);

    fs::create_directories(buildOutput);

    // TODO: Compile sub-units
    // TODO: Add sub folder to copy (For script for example)
    // TODO: Add native external libraries support

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
        std::cout << "Start module compilation : " << moduleRules.name << std::endl;

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

    std::vector<std::string> moduleList;
    for (const auto& modInf : unitRules.modules)
        moduleList.emplace_back(modInf.name);

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

    ExecutableCompileInfo eci(ci);
    eci.staticLibs = {};
    for (const auto& moduleRules : unitRules.modules)
        eci.staticLibs.emplace_back(buildOutput / "lib" / moduleRules.name);

    compiler->CompileExecutable(eci);
    delete compiler;
}
