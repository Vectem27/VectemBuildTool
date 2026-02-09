#pragma once

#if defined(_WIN32) || defined(_WIN64)

#include "Compiler/Compilation.h"
#include "ICompiler.h"

class ClangCompiler : public ICompiler
{
public:
    ClangCompiler() = default;

    virtual void CompileExecutable(const ExecutableCompileInfo& compileInfo) const override;
    virtual void CompileLibrary(const LibraryCompileInfo& compileInfo) const override;

protected:
    std::string GetCppVersionClangOption(CppVersion version) const;
};

class ClangCompilerFactory : public ICompilerFactory
{
public:
    ClangCompilerFactory() = default;

    virtual ICompiler* Create() const { return new ClangCompiler(); }
};

#endif