add_executable(VectemBuildTool 
    main.cpp

    Module/ModuleInfoReader.cpp
    Module/ModuleManager.cpp
    
    Compiler/LinuxCompiler.cpp

    Unit/UnitBuilder.cpp
    Unit/UnitConfigReader.cpp
    Unit/UnitRulesReader.cpp
)

target_include_directories(VectemBuildTool PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})