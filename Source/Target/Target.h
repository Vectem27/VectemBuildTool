#pragma once

#include <string>
#include <vector>
#include "Compiler/Compilation.h"

struct TargetRules
{
    std::string name;
    bool bAddDebugInfo = false;
    CVersion cVersion = CVersion::C17;
    CppVersion cppVersion = CppVersion::CPP20;
    int supportedPlatforms = 0xFF; // All platforms by default
    CompilationOptimisation optimisationType = CompilationOptimisation::STANDARD;
    FloatingPointModel floatingPointType = FloatingPointModel::PRECISE;
};
