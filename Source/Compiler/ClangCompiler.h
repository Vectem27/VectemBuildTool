#pragma once

#include <memory>

#include "Compiler/Compilation.h"
#include "ICompiler.h"
#include "CompileFileChangeManager.h"

class ClangCompiler : public ICompiler
{
public:
    ClangCompiler(std::shared_ptr<CompileFileChangeManager> fileChangeManager)
        : fileChangeManager(fileChangeManager) {}

    virtual bool CompileObjects(const CompileInfo& compileInfo) const override;
    virtual void ArchiveObjects(const ArchiveInfo& archiveInfo) const override;
    virtual void LinkBinary(const BinaryInfo& linkInfo) const override;

protected:
    std::vector<std::string> CreateCompileArgs(const CompileInfo& compileInfo, unsigned int fileLanguage) const;

    std::string GetCVersionClangOption(CVersion version) const;
    std::string GetCppVersionClangOption(CppVersion version) const;
    std::string GetFloatingPointClangOption(FloatingPointModel floatingPoint) const;
    std::string GetDebugInfoClangOption(bool bAddDebugInfo) const;

    std::shared_ptr<CompileFileChangeManager> fileChangeManager;
};

class ClangCompilerFactory : public ICompilerFactory
{
public:
    ClangCompilerFactory(std::shared_ptr<CompileFileChangeManager> fileChangeManager)
        : fileChangeManager(std::move(fileChangeManager)) {}

    virtual ICompiler* Create() const { return new ClangCompiler(fileChangeManager); }

private:
    std::shared_ptr<CompileFileChangeManager> fileChangeManager;
};