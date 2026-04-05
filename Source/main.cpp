#include <cstdlib>
#include <string>
#include <filesystem>
#include <vector>

#include <sol/sol.hpp>
#include <CLI/CLI11.hpp>

#include "BuildConfig/BuildConfigReader.h"
#include "Compiler/CompileCommandsExporter.h"
#include "Compiler/CompileFileChangeManager.h"
#include "Core/Logger.hpp"
#include "Compiler/ClangCompiler.h"
#include "Unit/UnitBuilder.h"

#include "Module/ModuleIncludeSolver.h"
#include "Module/ModuleGraphDependencySorter.h"

#include "Module/ModuleManager.h"

#include "Helper.h"

namespace fs = std::filesystem;

// TODO: Move this part of code
fs::path ResolveBuildOutput(const fs::path& unitRoot, const fs::path& configurationFile, const std::string& unitType,
                            const std::string& platform, const std::string& buildTarget)
{
    sol::state luaState;
    luaState.open_libraries(sol::lib::base, sol::lib::table, sol::lib::math, sol::lib::string, sol::lib::coroutine,
                            sol::lib::io);
    luaState.safe_script_file(configurationFile.string());

    BuildConfigReader buildConfigReader(luaState);
    const BuildConfig buildConfig = buildConfigReader.ReadBuildConfig();

    for (const auto& unitConfig : buildConfig.unitsInfo)
    {
        if (unitConfig.type == unitType)
            return unitConfig.buildDir / platform / buildTarget;
    }

    throw std::runtime_error("No config set for unit type: " + unitType);
}

void SetupBuildLogFile(const fs::path& buildOutput)
{
    fs::create_directories(buildOutput);

    const fs::path buildLogFile = buildOutput / "build.log";
    const fs::path backupLogFile = buildOutput / "build.backup.log";

    Logger::ClearOutputFile();

    if (fs::exists(backupLogFile))
        fs::remove(backupLogFile);

    if (fs::exists(buildLogFile))
        fs::rename(buildLogFile, backupLogFile);

    Logger::SetOutputFile(buildLogFile);
}

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
    std::string logLevel = "info";
    
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

    app.add_option("unit-type", unitType, "Unit type")
       ->required();

    app.add_option("build-target", buildTarget, "Target file path")
       ->required();

    /* Optional */

    app.add_option("-c, --config", confFile, "Build config file path")
       ->check(CLI::ExistingPath);

    app.add_option("-p, --platform", platform, "The build platform")
       ->check(CLI::ExistingPath);

    app.add_option("-l, --log-level", logLevel, "Minimum log level (critical, error, warning, info, debug, trace)");

    app.add_option("--dependancy", dependancyProjectsOptRes, "Projects dependancies (format : project-path unit-name unit-type build-target)")
        ->each([](const std::string& input) -> std::string {
            static int state = 0; // 0=path,1=unitName,2=unitType,3=buildTarget

            if (state == 0)
            {
                if (!fs::exists(fs::absolute(input)))
                {
                    throw CLI::ValidationError(
                        "--dependancy",
                        "Project path does not exist : " + input
                    );
                }
            }
            else if (state == 1)
            {
                if (input.empty())
                {
                    throw CLI::ValidationError(
                        "--dependancy",
                        "Unit name cannot be empty"
                    );
                }
            }
            else if (state == 2)
            {
                if (input.empty())
                {
                    throw CLI::ValidationError(
                        "--dependancy",
                        "Unit type cannot be empty"
                    );
                }
            }
            else if (state == 3)
            {
                if (input.empty())
                {
                    throw CLI::ValidationError(
                        "--dependancy",
                        "Build target cannot be empty"
                    );
                }
            }

            state = (state + 1) % 4;
            return input;
       });


    ///////////////////
    /* Parse Options */
    ///////////////////

    try
    {
        (app).parse(argc, argv);

        if (dependancyProjectsOptRes.size() % 4 != 0)
        {
            throw CLI::ValidationError(
                "--dependancy",
                "Each project dependancy must have project-path, unit-name, unit-type and build-target"
            );
        }
    }
    catch (const CLI::ParseError& e)
    {
        return app.exit(e);
    }

    const auto parsedLogLevel = TryParseLogLevel(logLevel);
    if (!parsedLogLevel.has_value())
    {
        Logger::Log(LogLevel::Error, "Invalid log level: %s", logLevel.c_str());
        return EXIT_FAILURE;
    }

    Logger::SetLevel(*parsedLogLevel);

    ////////////////////////////
    /* Modify Options Results */
    ////////////////////////////
 
    std::vector<ProjectDependancy> dependancyProjects;
 
    if (dependancyProjectsOptRes.size() % 4 != 0)
    {
        Logger::Log(LogLevel::Error, "Each project dependancy must have project-path, unit-name, unit-type and build-target");
        return EXIT_FAILURE;
    }
 
    for (size_t i = 0; i < dependancyProjectsOptRes.size(); i += 4)
    {
        dependancyProjects.push_back({
            .projectPath = fs::absolute(dependancyProjectsOptRes[i]),
            .unitName = dependancyProjectsOptRes[i + 1],
            .unitType = dependancyProjectsOptRes[i + 2],
            .buildTarget = dependancyProjectsOptRes[i + 3]
        });
    }

    if (!fs::is_directory(unitRoot))
    {
        Logger::Log(LogLevel::Error, "Unit root doesn't exist: %s", unitRoot.string().c_str());
        return EXIT_FAILURE;
    }

    if (confFile.empty())
    {
        confFile = unitRoot / "BuildConfig.lua";
        if (!fs::exists(confFile))
        {
            confFile = GetScriptDir() / "BuildConfig.lua";

            if (!fs::exists(confFile))
            {
                Logger::Log(
                    LogLevel::Error,
                    "--config is not set and the default config file doesn't exist. Default config file: %s",
                    confFile.string().c_str()
                );
                return EXIT_FAILURE;
            }
        }
    }
    else
    {
        confFile = fs::absolute(confFile);
        if (!fs::exists(confFile))
        {
            Logger::Log(LogLevel::Error, "Build config file doesn't exist: %s", confFile.string().c_str());
            return EXIT_FAILURE;
        }
    }

    if (!fs::is_regular_file(confFile))
    {
        Logger::Log(LogLevel::Error, "Build config path is not a valid file: %s", confFile.string().c_str());
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

    try
    {
        const fs::path buildOutput = ResolveBuildOutput(unitRoot, confFile, unitType, platform, buildTarget);
        SetupBuildLogFile(unitRoot / buildOutput);
        CompileCommandsExporter::Reset(unitRoot / buildOutput);
    }
    catch (const std::exception& exception)
    {
        Logger::Log(LogLevel::Critical, "%s", exception.what());
        return EXIT_FAILURE;
    }

    /////////////////
    /* Run Program */
    /////////////////

    try
    {
        Logger::Log(LogLevel::Info, "Starting build for unit '%s' (%s, target %s)", unitName.c_str(), unitType.c_str(), buildTarget.c_str());

        std::shared_ptr<CompileFileChangeManager> fileChangeManager = std::make_shared<CompileFileChangeManager>();

        ClangCompilerFactory compilerFactory = ClangCompilerFactory(fileChangeManager);
        ModuleIncludeSolver modIncSolver = ModuleIncludeSolver();
        ModuleGraphDependencySorter modDepSorter = ModuleGraphDependencySorter();
        ModuleManager moduleManager = ModuleManager();

        UnitBuilder builder = UnitBuilder(moduleManager, confFile, compilerFactory, modDepSorter, modIncSolver);

        BuildData buildData {
            .unitRoot = unitRoot,
            .unitName = unitName,
            .unitType = unitType,
            .buildTarget = buildTarget,
            .dependancyProjects = dependancyProjects,
            .configurationFile = confFile,
            .platform = platform
        };

        builder.BuildUnit(buildData);

        Logger::Log(LogLevel::Info, "Build completed successfully for unit '%s'", unitName.c_str());

        return EXIT_SUCCESS;
    }
    catch (const std::string& error)
    {
        Logger::Log(LogLevel::Critical, "Build failed: %s", error.c_str());
        return EXIT_FAILURE;
    }
    catch (const std::exception& exception)
    {
        Logger::Log(LogLevel::Critical, "Build failed: %s", exception.what());
        return EXIT_FAILURE;
    }

    return EXIT_FAILURE;
}