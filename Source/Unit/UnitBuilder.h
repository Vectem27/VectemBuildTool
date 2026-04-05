#pragma once


#include "../Compiler/ICompiler.h"
#include "Module/IModuleDependencySorter.h"
#include "Module/IModuleIncludeSolver.h"

#include "Unit/Unit.h"
#include "UnitBuilderBase.h"

class ITargetRulesReader;


class UnitBuilder : public UnitBuilderBase
{
protected:
    struct DependancyProcessingResult
    {
        std::string unitName;
        UnitCompilationType compilationType;
        std::filesystem::path buildOutputDir;
        std::filesystem::path libDir;
        std::filesystem::path binDir;
        std::vector<std::string> moduleNames;
    };
    
public:
    UnitBuilder(IModuleManager& moduleManager, 
        const std::filesystem::path& buildConfigFile, 
        const ICompilerFactory& compilerFactory, 
        const IModuleDependencySorter& moduleDepSorter,
        const IModuleIncludeSolver& moduleIncSolver
    );

    virtual void BuildUnit(const BuildData& buildData) override;

    virtual DependancyProcessingResult ProcessDependancyProject(const ProjectDependancy& dependancy, const BuildData& buildData);

protected:
    const ICompilerFactory& compilerFactory;
    const IModuleDependencySorter& moduleDepSorter;
    const IModuleIncludeSolver& moduleIncSolver;
};