#pragma once

#if defined(_WIN32) || defined(_WIN64)

#include "Compiler/Compilation.h"
#include "ICompiler.h"

class MsvcCompiler : public ICompiler
{
public:
    MsvcCompiler() = default;

    virtual void CompileExecutable(const ExecutableCompileInfo& compileInfo) const override;
    virtual void CompileLibrary(const LibraryCompileInfo& compileInfo) const override;

protected:
    std::string GetCVersionMsvcOption(CVersion version) const;
    std::string GetCppVersionMsvcOption(CppVersion version) const;
    std::string GetOptimisationMsvcOption(CompilationOptimisation optimisation) const;
    std::string GetFloatingPointMsvcOption(FloatingPointModel floatingPoint) const;
    std::string GetDebugInfoMsvcOption(bool bAddDebugInfo) const;
};

class MsvcCompilerFactory : public ICompilerFactory
{
public:
    MsvcCompilerFactory() = default;

    virtual ICompiler* Create() const { return new MsvcCompiler(); }
};

#endif