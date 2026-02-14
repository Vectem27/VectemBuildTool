#include "ModuleGraphDependencySorter.h"
#include "Module/Module.h"

#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <utility>
#include <functional>
#include <queue>

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
            do {
                w = S.top(); S.pop();
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


    std::unordered_set<std::pair<unsigned int, unsigned int>, 
        std::hash<unsigned long long>> seenDeps;

    std::vector<std::pair<unsigned int, unsigned int>> reducedDeps;

    for (const auto& dep : graph.dependancies)
    {
        unsigned int gFrom = moduleToGroup[dep.moduleName];
        unsigned int gTo   = moduleToGroup[dep.depModuleName];
        if (gFrom != gTo)
        {
            std::pair<unsigned int, unsigned int> p = {gFrom, gTo};
            if (!seenDeps.count(p))
            {
                reducedDeps.push_back(p);
                seenDeps.insert(p);
            }
        }
    }

    return ReductedModuleDepGraph{groups, reducedDeps};
}

SortedModulesGroups TopologicalSort(const ReductedModuleDepGraph& graph)
{
    SortedModulesGroups result;
    
    // Calculer le degré entrant de chaque groupe
    std::unordered_map<unsigned int, int> inDegree;
    std::unordered_map<unsigned int, std::vector<unsigned int>> adjList;
    
    // Initialiser tous les groupes avec un degré entrant de 0
    for (const auto& [groupId, modules] : graph.moduleGroups)
    {
        inDegree[groupId] = 0;
        adjList[groupId] = {};
    }
    
    // Construire la liste d'adjacence et calculer les degrés entrants
    for (const auto& [from, to] : graph.groupsDep)
    {
        adjList[from].push_back(to);
        inDegree[to]++;
    }
    
    // File pour les nœuds sans dépendance (degré entrant = 0)
    std::queue<unsigned int> queue;
    for (const auto& [groupId, degree] : inDegree)
    {
        if (degree == 0)
            queue.push(groupId);
    }
    
    // Algorithme de Kahn pour le tri topologique
    while (!queue.empty())
    {
        unsigned int current = queue.front();
        queue.pop();
        
        // Ajouter le groupe de modules au résultat
        auto it = graph.moduleGroups.find(current);
        if (it != graph.moduleGroups.end())
            result.push_back(it->second);
        
        // Réduire le degré entrant des voisins
        for (unsigned int neighbor : adjList[current])
        {
            inDegree[neighbor]--;
            if (inDegree[neighbor] == 0)
                queue.push(neighbor);
        }
    }
    
    // Vérifier s'il y a un cycle
    if (result.size() != graph.moduleGroups.size())
    {
        throw std::runtime_error("Cycle détecté dans le graphe de dépendances");
    }
    
    return result;
}

SortedModulesGroups ModuleGraphDependencySorter::Sort(std::vector<std::string> modulesName) const
{
    

    ModuleDepGraph moduleDepGraph;

    for (const auto& moduleName : modulesName)
        AddModuleAndDependencies(moduleName, moduleDepGraph.modules, moduleDepGraph.dependancies, modulesName);

    ReductedModuleDepGraph reducedGraph = BuildReductedGraph(moduleDepGraph);

    SortedModulesGroups res = TopologicalSort(reducedGraph);
    res.reverse();
    
    return res;
}

// Recursive function
void ModuleGraphDependencySorter::AddModuleAndDependencies(const std::string& moduleName, std::list<std::string>& modules,
                                                           std::list<ModuleDependency>& dependancies,
                                                           const std::vector<std::string>& unitModules) const
{
    if (!IsUnitModule(moduleName, unitModules)) // Add only unit modules
        return;

    if (!AddModule(moduleName, modules)) // Add only one time
        return;

    ModuleInfo moduleInfo = moduleManager.ResolveModuleInfo(moduleName);

    for (const auto& depModuleName : moduleInfo.publicModuleDependencies)
    {
        AddDependency(moduleName, depModuleName, dependancies);
        AddModuleAndDependencies(depModuleName, modules, dependancies, unitModules);
    }

    for (const auto& depModuleName : moduleInfo.privateModuleDependencies)
    {
        AddDependency(moduleName, depModuleName, dependancies);
        AddModuleAndDependencies(depModuleName, modules, dependancies, unitModules);
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
}
