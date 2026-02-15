#pragma once

#include "IModuleIncludeSolver.h"

class ModuleIncludeSolver : public IModuleIncludeSolver
{
public:
    virtual IncludesToAdd Resolve(const std::string& modulesName, const IModuleManager& moduleManager) const override;

protected:
    virtual void ResolveInto(const std::string& modulesName, const IModuleManager& moduleManager,
                             std::vector<std::string>& visitedModule, IncludesToAdd& includes, bool withPrivate) const;

    bool AddVisitedModule(const std::string& moduleName, std::vector<std::string>& visitedModule) const;

private:
};