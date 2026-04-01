#include "ModuleGraphDependencySorter.h"
#include "Core/Logger.hpp"
#include "Module/IModuleManager.h"
#include "Module/Module.h"

#include <algorithm>
#include <functional>
#include <queue>
#include <stack>
#include <set>
#include <vector>
#include <map>
#include <utility>

struct ModuleDepGraph
{
    std::list<std::string> modules;
    std::list<ModuleDependency> dependancies;
};

struct ReductedModuleDepGraph
{
    // Use vector indexed by group id for deterministic ordering
    std::vector<std::vector<std::string>> moduleGroups;
    std::vector<std::pair<unsigned int, unsigned int>> groupsDep;
};

ReductedModuleDepGraph BuildReductedGraph(const ModuleDepGraph& graph)
{
    // ----- Init -----
    std::map<std::string, std::vector<std::string>> adj;
    for (const auto& mod : graph.modules)
        adj[mod] = {};

    for (const auto& dep : graph.dependancies)
        adj[dep.moduleName].push_back(dep.depModuleName);

    // ----- Tarjan algorithme -----
    std::map<std::string, int> indexMap;
    std::map<std::string, int> lowLink;
    std::stack<std::string> S;
    std::set<std::string> onStack;
    int index = 0;
    unsigned int groupId = 0;

    std::map<std::string, unsigned int> moduleToGroup;
    std::vector<std::vector<std::string>> groups;

    std::function<void(const std::string&)> tarjan;
    tarjan = [&](const std::string& v)
    {
        indexMap[v] = index;
        lowLink[v] = index;
        index++;
        S.push(v);
        onStack.insert(v);

        for (const auto& w : adj[v])
        {
            if (indexMap.find(w) == indexMap.end())
            {
                tarjan(w);
                lowLink[v] = std::min(lowLink[v], lowLink[w]);
            }
            else if (onStack.count(w))
            {
                lowLink[v] = std::min(lowLink[v], indexMap[w]);
            }
        }

        if (lowLink[v] == indexMap[v])
        {
            std::vector<std::string> group;
            std::string w;
            do
            {
                w = S.top();
                S.pop();
                onStack.erase(w);
                moduleToGroup[w] = groupId;
                group.push_back(w);
            } while (w != v);
            groups.push_back(group);
            groupId++;
        }
    };

    for (const auto& mod : graph.modules)
    {
        if (indexMap.find(mod) == indexMap.end())
            tarjan(mod);
    }

    std::set<std::pair<unsigned int, unsigned int>> seenDeps;
    std::vector<std::pair<unsigned int, unsigned int>> reducedDeps;

    for (const auto& dep : graph.dependancies)
    {
        unsigned int gFrom = moduleToGroup[dep.moduleName];
        unsigned int gTo = moduleToGroup[dep.depModuleName];
        if (gFrom != gTo)
        {
            std::pair<unsigned int, unsigned int> p = {gFrom, gTo};
            if (seenDeps.insert(p).second)
                reducedDeps.push_back(p);
        }
    }

    return ReductedModuleDepGraph{groups, reducedDeps};
}

SortedModulesGroups TopologicalSort(const ReductedModuleDepGraph& graph)
{
    SortedModulesGroups result;
    const size_t N = graph.moduleGroups.size();
    std::vector<int> inDegree(N, 0);
    std::vector<std::vector<unsigned int>> adjList(N);

    for (const auto& p : graph.groupsDep)
    {
        unsigned int from = p.first;
        unsigned int to = p.second;
        if (from < N && to < N)
        {
            adjList[from].push_back(to);
            inDegree[to]++;
        }
    }

    // Use a min-heap so that when multiple groups have in-degree 0,
    // we always pick the smallest index first → deterministic ordering.
    std::priority_queue<unsigned int, std::vector<unsigned int>, std::greater<unsigned int>> pq;
    for (unsigned int i = 0; i < N; ++i)
        if (inDegree[i] == 0)
            pq.push(i);

    // Kahn's algorithm
    while (!pq.empty())
    {
        unsigned int current = pq.top();
        pq.pop();
        if (current < graph.moduleGroups.size())
            result.push_back(graph.moduleGroups[current]);

        for (unsigned int neighbor : adjList[current])
        {
            inDegree[neighbor]--;
            if (inDegree[neighbor] == 0)
                pq.push(neighbor);
        }
    }

    if (result.size() != graph.moduleGroups.size())
        throw std::runtime_error("There is a cylcle in the dependances group graph.");

    return result;
}

SortedModulesGroups ModuleGraphDependencySorter::Sort(std::vector<std::string> modulesName,
                                                      const IModuleManager& moduleManager) const
{
    // Sort input to ensure a canonical starting point regardless of caller order
    std::sort(modulesName.begin(), modulesName.end());

    ModuleDepGraph moduleDepGraph;

    for (const auto& moduleName : modulesName)
        AddModuleAndDependencies(moduleName, moduleDepGraph.modules, moduleDepGraph.dependancies, modulesName, moduleManager);

    ReductedModuleDepGraph reducedGraph = BuildReductedGraph(moduleDepGraph);

    SortedModulesGroups res = TopologicalSort(reducedGraph);
    return res;
}

// Recursive function
bool ModuleGraphDependencySorter::AddModuleAndDependencies(const std::string& moduleName, std::list<std::string>& modules,
                                                           std::list<ModuleDependency>& dependancies,
                                                           const std::vector<std::string>& unitModules,
                                                           const IModuleManager& moduleManager) const
{
    if (!IsUnitModule(moduleName, unitModules)) // Add only unit modules
        return false;

    if (!AddModule(moduleName, modules)) // Add only one time
        return false;

    ModuleInfo moduleInfo;
        
    try
    {
        moduleInfo = moduleManager.ResolveModuleInfo(moduleName);
    }
    catch (...) 
    {
        Logger::Log(LogLevel::Warning, "Module info can not be read after be added as dependancy.");
        return false;
    }

    for (const auto& depModuleName : moduleInfo.publicModuleDependencies)
    {
        AddModuleAndDependencies(depModuleName, modules, dependancies, unitModules, moduleManager);
        if (IsUnitModule(depModuleName, unitModules))
            AddDependency(moduleName, depModuleName, dependancies);
    }

    for (const auto& depModuleName : moduleInfo.privateModuleDependencies)
    {
        AddModuleAndDependencies(depModuleName, modules, dependancies, unitModules, moduleManager);
        if (IsUnitModule(depModuleName, unitModules))
            AddDependency(moduleName, depModuleName, dependancies);
    }

    return true;
}

bool ModuleGraphDependencySorter::AddModule(const std::string& moduleName, std::list<std::string>& modules) const
{
    for (const auto& modName : modules)
        if (modName == moduleName)
            return false;

    modules.emplace_back(moduleName);

    return true;
}

bool ModuleGraphDependencySorter::AddDependency(const std::string& moduleName, const std::string& depModuleName,
                                                std::list<ModuleDependency>& dependancies) const
{
    for (const auto& dep : dependancies)
        if (dep.moduleName == moduleName && dep.depModuleName == depModuleName)
            return false;

    dependancies.emplace_back(moduleName, depModuleName);

    return true;
}

bool ModuleGraphDependencySorter::IsUnitModule(const std::string& moduleName, const std::vector<std::string>& unitModules) const
{
    for (const auto& modName : unitModules)
        if (modName == moduleName)
            return true;

    return false;
}
