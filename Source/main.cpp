#include <cstdlib>
#include <iostream>
#include <string>
#include <filesystem>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>

#include "Compiler/Compilation.h"
#include "Compiler/ICompiler.h"
#include "Module/ModuleInfoReader.h"
#include "Compiler/ClangCompiler.h"

#include "Unit/UnitBuilder.h"

#include <sol/sol.hpp>

#include <CLI/CLI11.hpp>

namespace fs = std::filesystem;

/**
 * Arg 1 = Module dir
 * Arg 2 = Destination
 */
int main(int argc, char* argv[])
{
    CLI::App app{"Vectem Build Engine"};

    std::string fichier;
    bool verbose = false;


    std::string unitName;
    fs::path unitRoot;

    fs::path confFile;
    fs::path targetFile;

    app.add_option("unit-name", unitName, "Unit name")
       ->required();

    app.add_option("unit-root-path", unitRoot, "Unit root path")
       ->required()
       ->check(CLI::ExistingPath);

    app.add_option("build-config-path", confFile, "Build config file path")
       ->required()
       ->check(CLI::ExistingPath);

    app.add_option("target-file-path", targetFile, "Target file path")
       ->required()
       ->check(CLI::ExistingPath);

    try 
    {
        app.parse(argc, argv);
    } 
    catch (const CLI::ParseError &e) 
    {
        return app.exit(e);
    }

    unitRoot = fs::absolute(unitRoot);
    confFile = fs::absolute(confFile);
    targetFile = fs::absolute(targetFile);

    try
    {
        //fs::path sysroot = fs::absolute("Toolchains/linux/sysroot");

        ClangCompilerFactory compilerFactory = ClangCompilerFactory();

        UnitBuilder builder = UnitBuilder(compilerFactory);

        BuildData buildData {
            .unitName = unitName,
            .unitRoot = unitRoot,
            .configurationFile = confFile,
            .buildTargetFile = targetFile
        };

        builder.BuildUnit(buildData);

        return EXIT_SUCCESS;
    }
    catch (std::string e)
    {
        std::cerr << "Error : " << e << std::endl;
    }

    return EXIT_SUCCESS;
}