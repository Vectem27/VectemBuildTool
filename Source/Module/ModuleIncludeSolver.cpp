#include "ModuleIncludeSolver.h"

IncludesToAdd ModuleIncludeSolver::Resolve(const std::string& modulesName, const IModuleManager& moduleManager) const
{
    IncludesToAdd includes;
    std::vector<std::string> visitedModule;

    ResolveInto(modulesName, moduleManager, visitedModule, includes, true);

    return includes;
}

void ModuleIncludeSolver::ResolveInto(const std::string& modulesName, const IModuleManager& moduleManager,
                                      std::vector<std::string>& visitedModule, IncludesToAdd& includes, bool withPrivate) const
{
    if(!AddVisitedModule(modulesName, visitedModule))
        return;

    ModuleInfo modInf = moduleManager.ResolveModuleInfo(modulesName);

    auto GetFullIncPath = [&modInf](std::string subPath) -> std::filesystem::path {
        return modInf.directory / modInf.codeDir / subPath;
    };

    // Resolve public depencencies

    for (const auto& pubInc : modInf.publicIncludeDirectories)
        includes.emplace_back(GetFullIncPath(pubInc));

    for (const auto& dep : modInf.publicModuleDependencies)
        ResolveInto(dep, moduleManager, visitedModule, includes, false);

    // Resolve private depencencies

    if (!withPrivate)
        return;

    includes.emplace_back(modInf.directory / modInf.codeDir); // Adding code dir as private include directories.

    for (const auto& prvInc : modInf.privateIncludeDirectories)
        includes.emplace_back(GetFullIncPath(prvInc));

    for (const auto& dep : modInf.privateModuleDependencies)
        ResolveInto(dep, moduleManager, visitedModule, includes, false);    
}

bool ModuleIncludeSolver::AddVisitedModule(const std::string& moduleName, std::vector<std::string>& visitedModule) const
{
    for (const auto& visited : visitedModule)
        if (visited == moduleName)
            return false;

    visitedModule.emplace_back(moduleName);

    return true;
}
