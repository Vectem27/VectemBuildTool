
#include "UnitBuilder.h"

#include <filesystem>
#include <memory>

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

    std::vector<fs::path> dependencyBuildOutputs;
    dependencyBuildOutputs.reserve(dependanciesData.size());
    for (const auto& dependencyData : dependanciesData)
    {
        if (!dependencyData.buildOutputDir.empty())
            dependencyBuildOutputs.emplace_back(dependencyData.buildOutputDir);
    }

    fs::create_directories(buildOutput);

    auto getArchivePath = [](const fs::path& libDir, const std::string& outputName)
    {
        fs::path archivePath = libDir / ("lib" + outputName);
#ifdef _WIN32
        archivePath.replace_extension(".lib");
#else
        archivePath.replace_extension(".a");
#endif
        return archivePath;
    };

    // Start compilation

    std::unique_ptr<ICompiler> compiler(compilerFactory.Create());
    

    if (unitRules.compilationType == UnitCompilationType::StaticLibrary)
    {
        std::vector<fs::path> objectsFiles;
        bool shouldArchiveUnit = !fs::exists(getArchivePath(libOutput, buildData.unitName));

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
                .dependencyBuildOutputs = dependencyBuildOutputs,
                .cppFilesToCompile = cppFiles,
                .cFilesToCompile = cFiles,
                .includesPaths = includes,
                .bAddDebugInfo = targetRules.bAddDebugInfo,
                .cVersion = targetRules.cVersion,
                .cppVersion = targetRules.cppVersion,
                .optimisation = targetRules.optimisationType,
                .floatingPointModel = targetRules.floatingPointType,
            };
            const bool bObjectsUnchanged = compiler->CompileObjects(compInfo);
            if (!bObjectsUnchanged)
                shouldArchiveUnit = true;

            for (const auto& p : cppFiles)
                objectsFiles.emplace_back(objectOutput / p.filename().replace_extension(".o"));
            for (const auto& p : cFiles)
                objectsFiles.emplace_back(objectOutput / p.filename().replace_extension(".o"));
        }

        if (shouldArchiveUnit)
        {
            ArchiveInfo arInfo{
                .outputName = buildData.unitName,
                .libOututPath = GetStaticLibOutputDir(buildData),
                .objects = objectsFiles
            };
            compiler->ArchiveObjects(arInfo);
        }
        else
        {
            Logger::Log(LogLevel::Debug, "Skipping archive for unit '%s': no object file changed.", buildData.unitName.c_str());
        }

        return;
    }

    std::vector<std::string> moduleList;

    bool skipLinking = true;

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
            .dependencyBuildOutputs = dependencyBuildOutputs,
            .cppFilesToCompile = cppFiles,
            .cFilesToCompile = cFiles,
            .includesPaths = includes,
            .bAddDebugInfo = targetRules.bAddDebugInfo,
            .cVersion = targetRules.cVersion,
            .cppVersion = targetRules.cppVersion,
            .optimisation = targetRules.optimisationType,
            .floatingPointModel = targetRules.floatingPointType,
            .macros = {} // TODO: Set target macro
        };

        const auto& modAddMacro = moduleInfo.additionalMacro;

        compInfo.macros.insert(compInfo.macros.end(), modAddMacro.begin(), modAddMacro.end());

        const bool bObjectsUnchanged = compiler->CompileObjects(compInfo);

        std::vector<fs::path> objectsFiles;
        for (const auto& p : cppFiles)
            objectsFiles.emplace_back(objectOutput / p.filename().replace_extension(".o"));
        for (const auto& p : cFiles)
            objectsFiles.emplace_back(objectOutput / p.filename().replace_extension(".o"));

        const fs::path moduleArchivePath = getArchivePath(libOutput, moduleRules.name);
        if (!bObjectsUnchanged || !fs::exists(moduleArchivePath))
        {
            ArchiveInfo arInfo{
                .outputName = moduleRules.name,
                .libOututPath = GetStaticLibOutputDir(buildData),
                .objects = objectsFiles
            };
            compiler->ArchiveObjects(arInfo);

            skipLinking = false;
        }
        else
        {
            Logger::Log(LogLevel::Debug, "Skipping archive for module '%s': no object file changed.", moduleRules.name.c_str());
        }
    }

    if (skipLinking)
    {
        Logger::Log(LogLevel::Debug, "Skipping link edition for unit '%s': no module changed.", buildData.unitName.c_str());
        return;
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
    res.buildOutputDir = GetBuildOutputDir(dependancyBuildData);
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
