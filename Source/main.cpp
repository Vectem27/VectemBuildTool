#include <cstdlib>
#include <iostream>
#include <string>
#include <filesystem>
#include <vector>

#include <sol/sol.hpp>
#include <CLI/CLI11.hpp>

#include "Compiler/ClangCompiler.h"
#include "Unit/UnitBuilder.h"

namespace fs = std::filesystem;

// TODO: Made custom compile command from config file for extensibility)

/**
 * Arg 1 = Module dir
 * Arg 2 = Destination
 */
int main(int argc, char* argv[])
{
    //////////////////////
    /* Create Variables */
    //////////////////////

    CLI::App app{"Vectem Build Engine"};

    argv = app.ensure_utf8(argv);

    std::string unitName;
    fs::path unitRoot;

    fs::path confFile;
    std::string buildTarget;
    
    std::vector<std::string> dependancyProjectsOptRes;

    ////////////////////
    /* Create Options */
    ////////////////////

    /* Positional */

    app.add_option("unit-name", unitName, "Unit name")
       ->required();

    app.add_option("unit-root-path", unitRoot, "Unit root path")
       ->required()
       ->check(CLI::ExistingPath);

    app.add_option("build-target", buildTarget, "Target file path")
       ->required();

    /* Optional */

    app.add_option("-c, --config", confFile, "Build config file path")
       ->required()
       ->check(CLI::ExistingPath);

    app.add_option("--dependancy", dependancyProjectsOptRes, "Projects dependancies (format : project-path unit-type target-name)")
       ->each([](const std::string& input) -> std::string {
            static bool isProjectPath = true;

            if (isProjectPath)
            {
                if (!fs::exists(fs::absolute(input)))
                {
                    throw CLI::ValidationError(
                        "--dependancy",
                        "Project path does not exist : " + input
                    );
                }

            }
            else 
            {
                if (input.empty())
                {
                    throw CLI::ValidationError(
                        "--dependancy",
                        "Target name cannot be empty"
                    );
                }
            }

            isProjectPath = !isProjectPath;

           return input;
       });


    ///////////////////
    /* Parse Options */
    ///////////////////

    try
    {
        (app).parse(argc, argv);

        if (dependancyProjectsOptRes.size() % 2 != 0)
        {
            throw CLI::ValidationError(
                "--dependancy",
                "Each project dependancy must have a project path and a target name"
            );
        }
    }
    catch (const CLI ::ParseError& e)
    {
        return (app).exit(e);
    }

    ////////////////////////////
    /* Modify Options Results */
    ////////////////////////////

    struct ProjectDependancy
    {
        fs::path projectPath;
        std::string targetName;
    };

    std::vector<ProjectDependancy> dependancyProjects;

    for (size_t i = 0; i < dependancyProjectsOptRes.size(); i += 2)
    {
        dependancyProjects.push_back({
            .projectPath = fs::absolute(dependancyProjectsOptRes[i]),
            .targetName = dependancyProjectsOptRes[i + 1]
        });
    }

    unitRoot = fs::absolute(unitRoot);
    confFile = fs::absolute(confFile);

    /////////////////
    /* Run Program */
    /////////////////

    try
    {
        ClangCompilerFactory compilerFactory = ClangCompilerFactory();

        UnitBuilder builder = UnitBuilder(compilerFactory);

        BuildData buildData {
            .unitName = unitName,
            .unitRoot = unitRoot,
            .configurationFile = confFile,
            .buildTarget = buildTarget,
        };

#if defined(_WIN32)
        buildData.platform  = "Windows";
#else
        buildData.platform  = "Linux";
#endif

        builder.BuildUnit(buildData);

        return EXIT_SUCCESS;
    }
    catch (std::string e)
    {
        std::cerr << "Error : " << e << std::endl;
    }

    return EXIT_SUCCESS;
}