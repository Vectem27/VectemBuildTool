#pragma once

#include "Compiler/Compilation.h"
#include "ICompiler.h"

class LinuxCompiler : public ICompiler
{
public:
    LinuxCompiler(std::filesystem::path sysroot) : sysroot(sysroot) {}

    virtual void CompileExecutable(const ExecutableCompileInfo& compileInfo) const override;
    virtual void CompileLibrary(const LibraryCompileInfo& compileInfo) const override;


protected:
    std::string GetCppVersionClangOption(CppVersion version) const;

    std::filesystem::path sysroot;
};

class LinuxCompilerFactory : public ICompilerFactory
{
public:
    LinuxCompilerFactory(std::filesystem::path sysroot) : sysroot(sysroot) {}

    virtual ICompiler* Create() const { return new LinuxCompiler(sysroot); }

private:
    std::filesystem::path sysroot;
};