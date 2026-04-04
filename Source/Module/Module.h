#pragma once

#include <vector>
#include <filesystem>

#include "Types.h"
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

    std::vector<std::string> additionalStaticLib;
    std::vector<std::filesystem::path> additionalStaticLibDir;

    std::vector<Macro> additionalMacro;
};
