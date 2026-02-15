#include "ModuleGraphDependencySorter.h"
#include "Module/IModuleManager.h"
#include "Module/Module.h"

#include <functional>
#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <utility>

struct ModuleDepGraph
{
    std::list<std::string> modules;
    std::list<ModuleDependency> dependancies;
};

struct ReductedModuleDepGraph
{
    std::unordered_map<unsigned int, std::vector<std::string>> moduleGroups;
    std::vector<std::pair<unsigned int, unsigned int>> groupsDep;
};

ReductedModuleDepGraph BuildReductedGraph(const ModuleDepGraph& graph)
{
    // ----- Init -----
    std::unordered_map<std::string, std::vector<std::string>> adj;
    for (const auto& mod : graph.modules)
        adj[mod] = {};

    for (const auto& dep : graph.dependancies)
        adj[dep.moduleName].push_back(dep.depModuleName);

    // ----- Tarjan algorithme -----
    std::unordered_map<std::string, int> indexMap;
    std::unordered_map<std::string, int> lowLink;
    std::stack<std::string> S;
    std::unordered_set<std::string> onStack;
    int index = 0;
    unsigned int groupId = 0;

    std::unordered_map<std::string, unsigned int> moduleToGroup;
    std::unordered_map<unsigned int, std::vector<std::string>> groups;

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
            groups[groupId] = group;
            groupId++;
        }
    };

    for (const auto& mod : graph.modules)
    {
        if (indexMap.find(mod) == indexMap.end())
            tarjan(mod);
    }

    // ----- Custom hash for pairs -----
    struct PairHash
    {
        std::size_t operator()(const std::pair<unsigned int, unsigned int>& p) const
        {
            return std::hash<unsigned long long>()((static_cast<unsigned long long>(p.first) << 32) | p.second);
        }
    };

    std::unordered_set<std::pair<unsigned int, unsigned int>, PairHash> seenDeps;
    std::vector<std::pair<unsigned int, unsigned int>> reducedDeps;

    for (const auto& dep : graph.dependancies)
    {
        unsigned int gFrom = moduleToGroup[dep.moduleName];
        unsigned int gTo = moduleToGroup[dep.depModuleName];
        if (gFrom != gTo)
        {
            std::pair<unsigned int, unsigned int> p = {gFrom, gTo};
            if (seenDeps.insert(p).second) // insert retourne {iterator, bool}
            {
                reducedDeps.push_back(p);
            }
        }
    }

    return ReductedModuleDepGraph{groups, reducedDeps};
}

SortedModulesGroups TopologicalSort(const ReductedModuleDepGraph& graph)
{
    SortedModulesGroups result;

    std::unordered_map<unsigned int, int> inDegree;
    std::unordered_map<unsigned int, std::vector<unsigned int>> adjList;

    for (const auto& [groupId, modules] : graph.moduleGroups)
    {
        inDegree[groupId] = 0;
        adjList[groupId] = {};
    }

    for (const auto& [from, to] : graph.groupsDep)
    {
        adjList[from].push_back(to);
        inDegree[to]++;
    }

    std::queue<unsigned int> queue;
    for (const auto& [groupId, degree] : inDegree)
    {
        if (degree == 0)
            queue.push(groupId);
    }

    // Algorithme Kahn
    while (!queue.empty())
    {
        unsigned int current = queue.front();
        queue.pop();

        auto it = graph.moduleGroups.find(current);
        if (it != graph.moduleGroups.end())
            result.push_back(it->second);

        for (unsigned int neighbor : adjList[current])
        {
            inDegree[neighbor]--;
            if (inDegree[neighbor] == 0)
                queue.push(neighbor);
        }
    }

    if (result.size() != graph.moduleGroups.size())
    {
        throw std::runtime_error("There is a cylcle in the dependances group graph.");
    }

    return result;
}

SortedModulesGroups ModuleGraphDependencySorter::Sort(std::vector<std::string> modulesName,
                                                      const IModuleManager& moduleManager) const
{
    ModuleDepGraph moduleDepGraph;

    for (const auto& moduleName : modulesName)
        AddModuleAndDependencies(moduleName, moduleDepGraph.modules, moduleDepGraph.dependancies, modulesName, moduleManager);

    ReductedModuleDepGraph reducedGraph = BuildReductedGraph(moduleDepGraph);

    SortedModulesGroups res = TopologicalSort(reducedGraph);

    return res;
}

// Recursive function
void ModuleGraphDependencySorter::AddModuleAndDependencies(const std::string& moduleName, std::list<std::string>& modules,
                                                           std::list<ModuleDependency>& dependancies,
                                                           const std::vector<std::string>& unitModules,
                                                           const IModuleManager& moduleManager) const
{
    if (!IsUnitModule(moduleName, unitModules)) // Add only unit modules
        return;

    if (!AddModule(moduleName, modules)) // Add only one time
        return;

    ModuleInfo moduleInfo = moduleManager.ResolveModuleInfo(moduleName);

    for (const auto& depModuleName : moduleInfo.publicModuleDependencies)
    {
        AddDependency(moduleName, depModuleName, dependancies);
        AddModuleAndDependencies(depModuleName, modules, dependancies, unitModules, moduleManager);
    }

    for (const auto& depModuleName : moduleInfo.privateModuleDependencies)
    {
        AddDependency(moduleName, depModuleName, dependancies);
        AddModuleAndDependencies(depModuleName, modules, dependancies, unitModules, moduleManager);
    }
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
