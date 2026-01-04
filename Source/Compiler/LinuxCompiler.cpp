
#include "LinuxCompiler.h"

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

#include "Compiler/Compilation.h"

namespace fs = std::filesystem;

static void ExecuteCompilation(fs::path clangPath, std::vector<std::string> argStrings)
{
    // Créer le tableau de char*
    std::vector<char*> args;
    for (auto& s : argStrings)
        args.push_back(s.data());

    args.push_back(nullptr); // NULL final

    std::stringstream ss;
    ss<< "Starting compilation : ";
    for (const auto& arg : args)
        ss << " " << arg;
    std::cout << ss.str() << std::endl;

    pid_t pid = fork();
    if (pid == 0)
    {
        execvp(clangPath.c_str(), const_cast<char**>(args.data()));
        perror("execvp");        
    }
    else 
    {
        wait(nullptr);
        std::cout << "Command finished\n";
    }
}

static void ExecuteLinking(fs::path arPath, std::vector<std::string> argStrings)
{
    std::vector<char*> args;
    for (auto& s : argStrings)
        args.push_back(s.data());

    args.push_back(nullptr); // NULL final

    std::stringstream ss;
    ss<< "Starting linking : ";
    for (const auto& arg : args)
        ss << " " << arg;
    std::cout << ss.str() << std::endl;

    pid_t pid = fork();
    if (pid == 0)
    {
        execvp(arPath.c_str(), const_cast<char**>(args.data()));
        perror("execvp");        
    }
    else 
    {
        wait(nullptr);
        std::cout << "Command finished\n";
    }
}

void LinuxCompiler::CompileExecutable(const ExecutableCompileInfo& compileInfo) const
{
    std::vector<std::string> argStrings;

    fs::path clangPath = (sysroot / "clang/bin/clang++");

    // Set command argument
    argStrings.push_back(clangPath);

    // Add files to compile
    for (const auto& file : compileInfo.filesToCompile)
        argStrings.push_back(file);

    // Add include paths
    for (const auto& include : compileInfo.includesPaths)
        argStrings.push_back("-I" + include.string());

    // Set cpp version
    argStrings.push_back(GetCppVersionClangOption(compileInfo.cppVersion));
    
    argStrings.push_back("-nostdinc++"); // Ignore c++ standard includes
    argStrings.push_back("-isystem"); // Include system
    argStrings.push_back(sysroot / "clang/include/c++/v1"); // C++ includes
    argStrings.push_back("-isystem"); // Include system
    argStrings.push_back(sysroot / "clang/include/x86_64-unknown-linux-gnu/c++/v1"); // C++ includes

    // Set everything to static link
    for (const auto& libPath : compileInfo.staticLibs)
        argStrings.push_back("-L" + (libPath.parent_path()).string()); 

    argStrings.push_back("-Wl,--start-group");
    for (const auto& libPath : compileInfo.staticLibs)
        argStrings.push_back("-l" + (libPath.filename()).string()); 
    argStrings.push_back("-Wl,--end-group");

    argStrings.push_back("-L" + (sysroot / "clang/lib").string()); // C++ library
    argStrings.push_back("-static-libstdc++");
    argStrings.push_back("-static-libgcc");
    argStrings.push_back("-Wl,-Bstatic");
    argStrings.push_back("-lc++");
    argStrings.push_back("-lc++abi");
    argStrings.push_back("-lunwind");


    // Set c to dynamic link
    argStrings.push_back("-Wl,-Bdynamic");
    argStrings.push_back("-lc");

    argStrings.push_back("-o");
    argStrings.push_back(compileInfo.buildOutputPath / "bin" / compileInfo.outputName);

    fs::create_directories(compileInfo.buildOutputPath / "bin");

    ExecuteCompilation(clangPath, argStrings);
}

void LinuxCompiler::CompileLibrary(const LibraryCompileInfo& compileInfo) const
{

    fs::path clangPath = (sysroot / "clang/bin/clang++");
    fs::path arPath = (sysroot / "clang/bin/llvm-ar");
    fs::path objDir = compileInfo.buildOutputPath / "obj";
    fs::path libDir = compileInfo.buildOutputPath / "lib";

    std::vector<fs::path> objects;
    fs::create_directories(objDir);
    fs::create_directories(libDir);

    // Add files to compile
    for (const auto& file : compileInfo.filesToCompile)
    {
        std::vector<std::string> argStrings;

        // Set command argument
        argStrings.push_back(clangPath);

        argStrings.push_back(GetCppVersionClangOption(compileInfo.cppVersion));

        // Add include paths
        for (const auto& include : compileInfo.includesPaths)
            argStrings.push_back("-I" + include.string());

            
        argStrings.push_back("-c");

        argStrings.push_back(file);

        argStrings.push_back("-o");

        fs::path objectFilename = objDir / file.filename();
        objectFilename.replace_extension(".o");

        argStrings.push_back(objectFilename);

        objects.push_back(objectFilename);

        ExecuteCompilation(clangPath, argStrings);
    }

    std::vector<std::string> argStrings;
    argStrings.push_back(arPath);
    argStrings.push_back("rcs");
    argStrings.push_back(libDir / ("lib" + compileInfo.outputName + ".a"));

    for (auto& obj : objects)
        argStrings.push_back(obj);

    ExecuteLinking(arPath, argStrings);
}

std::string LinuxCompiler::GetCppVersionClangOption(CppVersion version) const
{
    std::string opt("-std=");
    switch (version)
    {
    case CppVersion::CPP_11:
        return opt + "c++11";
        case CppVersion::CPP_14:
            return opt + "c++14";
        case CppVersion::CPP_17:
            return opt + "c++17";
        case CppVersion::CPP_20:
            return opt + "c++20";
        case CppVersion::CPP_23:
            return opt + "c++23";
        default:
            return opt + "c++11";
    }
}