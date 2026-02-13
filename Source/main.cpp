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

    fs::path unitRoot;
    std::string unitName;
    std::string unitType;
    std::string buildTarget;

    fs::path confFile;
    std::string platform;
    
    std::vector<std::string> dependancyProjectsOptRes;

    ////////////////////
    /* Create Options */
    ////////////////////

    /* Positional */

    app.add_option("unit-root-path", unitRoot, "Unit root path")
       ->required()
       ->check(CLI::ExistingPath);

    app.add_option("unit-name", unitName, "Unit name")
       ->required();

    app.add_option("unit-type", unitType, "Unit name")
       ->required();

    app.add_option("build-target", buildTarget, "Target file path")
       ->required();

    /* Optional */

    app.add_option("-c, --config", confFile, "Build config file path")
       ->check(CLI::ExistingPath);

    app.add_option("-p, --platform", platform, "The build platform")
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

    if (!fs::is_directory(unitRoot))
    {
        std::cerr << "Error: Unit root doesn't exists." << std::endl;
        return EXIT_FAILURE;
    }

    if (confFile.empty())
    {
        confFile = unitRoot / "BuildConfig.lua";
        if (!fs::exists(confFile))
        {
            std::cerr << "Error: --config is not set and default config file doesn't exist. (Default config file : " << confFile << ")" << std::endl;
            return EXIT_FAILURE;
        }
    }
    else
    {
        confFile = fs::absolute(confFile);
        if (!fs::exists(confFile))
        {
            std::cerr << "Error: Build config file doesn't exist." << std::endl;
            return EXIT_FAILURE;
        }
    }

    if (!fs::is_regular_file(confFile))
    {
        std::cerr << "Error: Build config path is not a valid file" << std::endl;
        return EXIT_FAILURE;
    }

    // TODO: Check other inputs.
    // TODO: Check for valide platform

    if(platform.empty())
    {
#if defined(_WIN32)
        platform  = "Windows";
#else
        platform  = "Linux";
#endif
    }

    /////////////////
    /* Run Program */
    /////////////////

    try
    {
        ClangCompilerFactory compilerFactory = ClangCompilerFactory();

        UnitBuilder builder = UnitBuilder(compilerFactory);

        BuildData buildData {
            .unitRoot = unitRoot,
            .unitName = unitName,
            .unitType = unitType,
            .buildTarget = buildTarget,
            .configurationFile = confFile,
            .platform = platform
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