#pragma once


#include "../Compiler/ICompiler.h"
#include "Module/IModuleDependencySorter.h"
#include "Module/IModuleIncludeSolver.h"

#include "UnitBuilderBase.h"

class ITargetRulesReader;


class UnitBuilder : public UnitBuilderBase
{
protected:
    struct DependancyProcessingResult
    {
        std::filesystem::path libDir;
    };
    
public:
    UnitBuilder(IModuleManager& moduleManager, const ICompilerFactory& compilerFactory, const IModuleDependencySorter& moduleDepSorter,
                const IModuleIncludeSolver& moduleIncSolver);

    virtual void BuildUnit(const BuildData& buildData) override;

    virtual DependancyProcessingResult ProcessDependancyProject(const ProjectDependancy& dependancy, const BuildData& buildData);

protected:
    const ICompilerFactory& compilerFactory;
    const IModuleDependencySorter& moduleDepSorter;
    const IModuleIncludeSolver& moduleIncSolver;
};