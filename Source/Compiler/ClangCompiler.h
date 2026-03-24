#pragma once

#include "Compiler/Compilation.h"
#include "ICompiler.h"

class ClangCompiler : public ICompiler
{
public:
    ClangCompiler() = default;

    virtual void CompileExecutable(const ExecutableCompileInfo& compileInfo) const override;
    virtual void CompileLibrary(const LibraryCompileInfo& compileInfo) const override;

protected:
    std::string GetCVersionClangOption(CVersion version) const;
    std::string GetCppVersionClangOption(CppVersion version) const;
    std::string GetOptimisationClangOption(CompilationOptimisation optimisation) const;
    std::string GetFloatingPointClangOption(FloatingPointModel floatingPoint) const;
    std::string GetDebugInfoClangOption(bool bAddDebugInfo) const;
};

class ClangCompilerFactory : public ICompilerFactory
{
public:
    ClangCompilerFactory() = default;

    virtual ICompiler* Create() const { return new ClangCompiler(); }
};