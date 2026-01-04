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
#include "Compiler/LinuxCompiler.h"

#include "Unit/UnitBuilder.h"

#include <sol/sol.hpp>

namespace fs = std::filesystem;

/**
 * Arg 1 = Module dir
 * Arg 2 = Destination
 */
int main(int argc, char* argv[])
{
    try
    {
        if (argc != 5)
            throw std::string("Bad arg count");


        fs::path unitName = argv[1];
        fs::path unitRoot = argv[2];

        fs::path confFile = argv[3];
        fs::path targetFile = argv[4];

        fs::path sysroot = fs::absolute("Toolchains/linux/sysroot");

        LinuxCompilerFactory compilerFactory = LinuxCompilerFactory(sysroot);

        UnitBuilder builder = UnitBuilder(compilerFactory);

        BuildData buildData {
            .unitName = unitName,
            .unitRoot = unitRoot,
            .configurationFile = confFile,
            .buildTargetFile = targetFile
        };

        builder.BuildUnit(buildData);

        return EXIT_SUCCESS;

        std::string type = argv[1];
        fs::path moduleDir = argv[2];
        fs::path buildDest = argv[3];

        fs::path moduleConfigPath = moduleDir / "Module.lua";
        fs::path moduleSourceDir = moduleDir / "Source";
        fs::path moduleBuildDir = moduleDir / "Build";

        ModuleInfo info;
        {
            // Start lua
            sol::state lua; // Auto close at the scope end
            lua.open_libraries(
                sol::lib::base,
                sol::lib::table,
                sol::lib::math,
                sol::lib::string,
                sol::lib::coroutine
            );
            ModuleInfoReader reader(lua);
            info = reader.ReadInfo(moduleConfigPath);
        }

        std::cout << "Module name : " << info.name << std::endl;
        for (const auto& dir : info.publicIncludeDirectories)
            std::cout << "-public dir : " << dir << std::endl;

        std::vector<fs::path> cppFiles;

        for (auto& p : fs::recursive_directory_iterator(moduleDir / "Source")) 
        {
        if (p.is_regular_file()) { // vérifier que c'est un fichier
            if (p.path().extension() == ".cpp" || p.path().extension() == ".cxx") {
                cppFiles.push_back(p.path());
            }
        }

        fs::create_directories(moduleBuildDir);


        LinuxCompiler linux_comp(sysroot);

        CompileInfo ci = {};
        ci.outputName = info.name;
        ci.buildOutputPath = moduleBuildDir;
        ci.filesToCompile = cppFiles;
        ci.includesPaths = {};
        ci.cppVersion = CppVersion::CPP_20;
        ci.optimisation = CompilationOptimisation::OPTIMIZED;

        if (type == "exe")
        {
            ExecutableCompileInfo eci(ci);
            eci.staticLibs = {"/home/vectem/Projets/Perso/VectemEngine/VectemBuildTool/Test/ModuleTest/Build/lib/ModuleTest"};
            linux_comp.CompileExecutable(eci);
        }
        else if (type == "lib")
        {
            LibraryCompileInfo lci(ci);
            lci.linkType = LinkType::STATIC;

            linux_comp.CompileLibrary(lci);
        }
        else
            throw std::string("Unknown tyoe : " + type);

        return EXIT_SUCCESS;
    }
    }
    catch (std::string e)
    {
        std::cerr << "Error : " << e << std::endl;
    }

    return EXIT_SUCCESS;
}