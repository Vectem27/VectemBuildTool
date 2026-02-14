#pragma once

#include <string>
#include <vector>
#include <filesystem>

enum class LinkingType
{
    STATIC,
    DYNAMIC,
    DYNAMIC_LOADING
};

struct ModuleInfo
{
    std::string name;
    std::filesystem::path directory;
    std::string codeDir; // Relative to root directory

    /* Include directories (relative from the module source root)*/
    std::vector<std::string> publicIncludeDirectories;
    std::vector<std::string> privateIncludeDirectories;

    std::vector<std::string> publicModuleDependencies;
    std::vector<std::string> privateModuleDependencies;

    LinkingType linkingType;
};
