add_executable(VectemBuildTool 
    main.cpp

    Module/ModuleInfoReader.cpp
    Module/ModuleManager.cpp
    
    Compiler/ClangCompiler.cpp
    Compiler/MsvcCompiler.cpp

    Unit/UnitBuilder.cpp
    Unit/UnitRulesReader.cpp

    BuildConfig/BuildConfigReader.cpp

    Target/TargetRulesReader.cpp
)

target_include_directories(VectemBuildTool PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})