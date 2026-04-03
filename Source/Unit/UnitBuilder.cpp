
#include "UnitBuilder.h"

#include <filesystem>

#include <sol/sol.hpp>
#include <vector>

#include "Core/Logger.hpp"
#include "Compiler/ICompiler.h"
#include "Module/IModuleDependencySorter.h"
#include "Module/IModuleManager.h"
#include "Module/Module.h"
#include "Unit/Unit.h"
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
    auto libOutput = GetStaticLibOutputDir(buildData);
    auto objectOutput = GetObjectOutputDir(buildData);
    auto binOutput = GetBinaryOutputDir(buildData);

    fs::create_directories(buildOutput);

    // Start compilation

    ICompiler* compiler = compilerFactory.Create();
    

    if (unitRules.compilationType == UnitCompilationType::StaticLibrary)
    {
        std::vector<fs::path> objectsFiles;

        for (const auto& moduleRules : unitRules.modules)
        {
            ModuleInfo moduleInfo = moduleManager.ResolveModuleInfo(moduleRules.name);

            IncludesToAdd includes = moduleIncSolver.Resolve(moduleInfo.name, moduleManager);

            std::vector<fs::path> cppFiles;
            std::vector<fs::path> cFiles;
            for (auto& p : fs::recursive_directory_iterator(moduleInfo.directory / moduleInfo.codeDir))
            {
                if (!p.is_regular_file())
                    continue;

                if (p.path().extension() == ".cpp" || p.path().extension() == ".cxx")
                    cppFiles.emplace_back(std::move(p.path()));
                else if (p.path().extension() == ".c")
                    cFiles.emplace_back(std::move(p.path()));
            }

            // Compile
            Logger::Log(LogLevel::Info, "Start module compilation: %s", moduleRules.name.c_str());

            CompileInfo compInfo = {
                .buildOutputPath = buildOutput,
                .objectOutputPath = objectOutput,
                .cppFilesToCompile = cppFiles,
                .cFilesToCompile = cFiles,
                .includesPaths = includes,
                .bAddDebugInfo = targetRules.bAddDebugInfo,
                .cVersion = targetRules.cVersion,
                .cppVersion = targetRules.cppVersion,
                .optimisation = targetRules.optimisationType,
                .floatingPointModel = targetRules.floatingPointType,
            };
            compiler->CompileObjects(compInfo);

            for (const auto& p : cppFiles)
                objectsFiles.emplace_back(objectOutput / p.filename().replace_extension(".o"));
        }

        ArchiveInfo arInfo{
            .outputName = buildData.unitName,
            .libOututPath = GetStaticLibOutputDir(buildData),
            .objects = objectsFiles
        };
        compiler->ArchiveObjects(arInfo);

        return;
    }

    std::vector<std::string> moduleList;

    for (const auto& moduleRules : unitRules.modules)
    {
        moduleList.push_back(moduleRules.name);
        ModuleInfo moduleInfo = moduleManager.ResolveModuleInfo(moduleRules.name);
        IncludesToAdd includes = moduleIncSolver.Resolve(moduleInfo.name, moduleManager);

        std::vector<fs::path> cppFiles;
        std::vector<fs::path> cFiles;
        for (auto& p : fs::recursive_directory_iterator(moduleInfo.directory / moduleInfo.codeDir))
        {
            if (!p.is_regular_file())
                continue;

            if (p.path().extension() == ".cpp" || p.path().extension() == ".cxx")
                cppFiles.emplace_back(std::move(p.path()));
            else if (p.path().extension() == ".c")
                cFiles.emplace_back(std::move(p.path()));
        }

        // Compile
        Logger::Log(LogLevel::Info, "Start module compilation: %s", moduleRules.name.c_str());

        CompileInfo compInfo = {
            .buildOutputPath = buildOutput,
            .objectOutputPath = objectOutput,
            .cppFilesToCompile = cppFiles,
            .cFilesToCompile = cFiles,
            .includesPaths = includes,
            .bAddDebugInfo = targetRules.bAddDebugInfo,
            .cVersion = targetRules.cVersion,
            .cppVersion = targetRules.cppVersion,
            .optimisation = targetRules.optimisationType,
            .floatingPointModel = targetRules.floatingPointType,
        };
        compiler->CompileObjects(compInfo);

        std::vector<fs::path> objectsFiles;
        for (const auto& p : cppFiles)
            objectsFiles.emplace_back(objectOutput / p.filename().replace_extension(".o"));

        ArchiveInfo arInfo{
            .outputName = moduleRules.name,
            .libOututPath = GetStaticLibOutputDir(buildData),
            .objects = objectsFiles
        };
        compiler->ArchiveObjects(arInfo);
    }

    BinaryInfo binInfo = {
        .binaryType = BinaryType::Executable,
        .binaryOutputPath = GetBinaryOutputDir(buildData),
        .outputName = buildData.unitName,
        .bAddDebugInfo = targetRules.bAddDebugInfo,
        .optimisation = targetRules.optimisationType
    };


    for (const auto& dep : dependanciesData)
    {
        if (dep.compilationType == UnitCompilationType::StaticLibrary)
        {
            moduleList.insert(moduleList.end(), dep.moduleNames.begin(), dep.moduleNames.end());
            binInfo.libPaths.emplace_back(dep.libDir);
        }
        else if (dep.compilationType == UnitCompilationType::DynamicLibrary)
        {
            binInfo.libPaths.emplace_back(dep.binDir);
            binInfo.dynamicLibsToLink.emplace_back(dep.unitName);

            try 
            {
                if (!fs::exists(binOutput))
                    fs::create_directories(binOutput);

                fs::path source = (dep.binDir / ("lib" + dep.unitName)).replace_extension(".so");
                fs::path destination = (binOutput / ("lib" + dep.unitName)).replace_extension(".so");
#ifdef _WIN32
                source.replace_filename(dep.unitName + ".dll");
                destination.replace_extension(dep.unitName + ".dll");
#endif

                fs::copy_file(source, destination, fs::copy_options::overwrite_existing);
                Logger::Log(LogLevel::Info, 
                    "'%s' dynamic library was successfully copied to the build destination.", 
                    dep.unitName.c_str());
            } 
            catch (const fs::filesystem_error& e) 
            {
                Logger::Log(LogLevel::Error, 
                    "Failed to copy the unit dynamic library '%s' to the build destination.", 
                    dep.unitName.c_str());
                throw UnitBuilderException(e.what());
            }
        }
        else
        {
            std::string errorMsg = "Can not link project dependancy : '" + dep.unitName + "', the unit is an executable.";
            Logger::Log(LogLevel::Error, errorMsg.c_str());
            throw UnitBuilderException(errorMsg);
        }
    }
    

    SortedModulesGroups sortedModules = moduleDepSorter.Sort(moduleList, moduleManager);


    if (unitRules.compilationType == UnitCompilationType::DynamicLibrary)
        binInfo.binaryType = BinaryType::DynamicLibrary;

    for (const auto& group : sortedModules)
    {
        std::vector<std::filesystem::path> groupPaths;
        for (const auto& module : group)
            groupPaths.emplace_back(libOutput / module);
        binInfo.staticLibsToLink.emplace_back(groupPaths);
    }

    for (const auto& group : sortedModules)
    {
        for (const auto& module : group)
        {
            auto staticLibs = moduleManager.ResolveModuleInfo(module).additionalStaticLib;
            for (const auto& lib : staticLibs)
                binInfo.staticLibsToLink.emplace_back(std::vector<fs::path>{lib});

            auto staticLibDirs = moduleManager.ResolveModuleInfo(module).additionalStaticLibDir;
            for (const auto& lib : staticLibs)
                binInfo.libPaths.emplace_back(lib);
        }
    }

    binInfo.libPaths.emplace_back(libOutput);

    compiler->LinkBinary(binInfo);
    
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
        .buildTarget = dependancy.buildTarget,
        .dependancyProjects = {},
        .configurationFile = buildData.configurationFile,
        .platform = buildData.platform
    };

    res.unitName = dependancyBuildData.unitName;
    res.binDir = GetBinaryOutputDir(dependancyBuildData);
    res.libDir = GetStaticLibOutputDir(dependancyBuildData);

    UnitConfig unitConfig = GetUnitConfig(dependancyBuildData.unitType);

    std::filesystem::path unitRulesFile = GetUnitRulesFile(dependancyBuildData);

    luaState.safe_script_file(unitRulesFile.string());

    auto unitRules = FetchUnitRules(luaState, dependancyBuildData);

    res.compilationType = unitRules.compilationType;
    for (const auto& moduleInfo : unitRules.modules)
        res.moduleNames.push_back(moduleInfo.name);

    ReadModulesrules(dependancyBuildData, unitConfig, unitRulesFile, unitRules.modules);

    return res;
}
