#pragma once

#include <string>
#include <vector>

struct ModuleInfo
{
    std::string name;
    std::string directory;

    /* Include directories (relative from the module source root)*/
    std::vector<std::string> publicIncludeDirectories;
};
