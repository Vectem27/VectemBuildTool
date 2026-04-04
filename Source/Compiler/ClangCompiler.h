#pragma once

#include "Compiler/Compilation.h"
#include "ICompiler.h"

class ClangCompiler : public ICompiler
{
public:
    ClangCompiler() = default;

    virtual void CompileObjects(const CompileInfo& compileInfo) const override;
    virtual void ArchiveObjects(const ArchiveInfo& archiveInfo) const override;
    virtual void LinkBinary(const BinaryInfo& linkInfo) const override;

protected:
    std::vector<std::string> CreateCompileArgs(const CompileInfo& compileInfo, unsigned int fileLanguage) const;

    std::string GetCVersionClangOption(CVersion version) const;
    std::string GetCppVersionClangOption(CppVersion version) const;
    std::string GetFloatingPointClangOption(FloatingPointModel floatingPoint) const;
    std::string GetDebugInfoClangOption(bool bAddDebugInfo) const;
};

class ClangCompilerFactory : public ICompilerFactory
{
public:
    ClangCompilerFactory() = default;

    virtual ICompiler* Create() const { return new ClangCompiler(); }
};