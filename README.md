# Vectem Build Tool

## Presentation

Vectem build tool is a customisable C++ module based build system.

## Use

Pattern command :

```bash
VectemBuildTool --config BuildConfig.lua --project ProjectDir/ TargetName --dependancy-project DepProjectDir/ --build-type Debug
```

Simple project example :

```bash
VectemBuildTool --config BuildConfig.lua --project ExampleProject/ --target Test --build-type Release
```

Project with engine example :

```bash
VectemBuildTool --config BuildConfig.lua --project Project/ --target Test --dependancy-projects Engine/ --dependancy-targets Test
```

## Project Composition

A project needs 4 types of configuration files :

1) The build configuration file
2) The target rules file
3) The unit rules files
4) The modules rules files

These files are lua scripts, so dynamic data can be set.
Script can communicate, indeed build configuration script and target rules script are executed before every unit rules script and module rules scripts.
Note : the unit rules script is executed before every module rules sripts too.

A lot of project properties might be cutomised like folder structure or some rules scripts syntaxe.

### Modules

Module is a code container, modules can have dependancies with other module.
Modules can override some compilation rules, so some modules can be compiled with different optimisation levels or with different language versions.

A module can't be compiled alone, they needs to be compiled in a [unit](#units).

### Units

'Units' or 'build units' contains [modules](#modules).
The unit can be compiled alone as a library or executable.
A unit can contains sub-units according to the [configuration file](#build-confuguration), for example your project plugins can be sub-units.

Units can have dependancies with other units or sub-units.

Separate units dependancies are gave in the build command and should be compiled separately, them build rules and them config can be different as the project.using them.

Sub-units can be compile with the main unit (optional).

### Target

A project can required different target (e.g. Release, Debug).
Target can change build rules and compilation options.
Most of these rules can be overrided for single modules.

Sub-units share the same target as the main one.

### Build Configuration {#build-confuguration}

The build configuration define units structure and some units rules.
It defines possible units types, them folder structure, them sub-units and some defaults rules.

## Scripts

Every scripts should contains specific lua tables with some required fields.

### Build Configuration Script {#build-confuguration-script}

By default, the build will use the file named 'BuildConfig.lua' located at the project root.
If this file doesn't exist, the internal config script will be used.
You can the command option '--config' to use an other config file.

The build configuration script can define some functions and utilities for every rules scripts.
However, it must define a table named 'UnitsConfig' used to define the project structure.

The table 'UnitsConfig' contains units rules.
To define a unit rule, add an table field named as the unit type name.

The unit table should contains some fields :

- 'ModulesDir' an array containing relatives directory names. (Can't be empty)
- 'ModuleRootName'  The module root directory name.
- 'ModuleFileName'  The module rules script file name.
- 'ModuleClassName' The module rules script table name.
- 'BuildDir' The unit build directory. (relative to the unit root).
- 'SubUnits' The sub-units config table. (Can be empty)

For 'ModuleRootName', 'ModuleFileName' and 'ModuleClassName' you can add the macro '${ModuleName}' in the string, it will be replaced by the module name.

'SubUnits' is an array containing tables, this table required these fields :

- 'Dir' The sub-units dir
- 'bRecursive'   A boolean, if true search units recursively inside the dir.
- 'UnitType' The units types
- 'UnitRootName' The unit root dir name
- 'UnitFileName' The unit script file name dir

For 'UnitRootName' and 'UnitFileName' you can add the macro '${UnitName}' in the string, it will be replaced by the sub-unit name.

Non sub-units require some other fields :

- 'TargetsDir' The targets directory.
- 'TargetFileName'  The target rules script file name.
- 'TargetClassName' The target rules script table name.

For 'TargetFileName' and 'TargetClassName' you can add the macro '${TargetName}' in the string, it will be replaced by the target name.

Example :

```lua
UnitsConfig = {
    Program = {
        ModulesDir      = { "Modules" },
        ModuleRootName  = "${ModuleName}Module",
        ModuleFileName  = "${ModuleName}.Module.lua",
        ModuleClassName = "${ModuleName}Rules", -- Not implemented

        TargetsDir = "Targets",
        TargetFileName  = "${TargetName}.Target.lua",
        TargetClassName = "${TargetName}TagetRules",

        BuildDir = "Build",

        SubUnits = {
            {
                Dir = "Plugins",
                UnitType = "Plugin",
                UnitRootName = "${UnitName}",
                UnitFileName = "${UnitName}.Plugin.lua",
                bRecursive=true
            }
        }
    },

    Plugin = {
        ModulesDir      = { "Modules" },
        ModuleRootName  = "${ModuleName}",
        ModuleFileName  = "${ModuleName.Build.lua}",
        ModuleClassName = "${ModuleName}Rules",

        BuildDir = "Build",
        
        SubUnits = {}
    }
}
```

### Target Rules Script {#target-rules-script}

The target table name is defined in the unit config.
Inside this table some fields are required :

- 'bAddDebugInfo' Boolean defining if we should compile with debug info.
- 'CVersion' A string representing the C version.
- 'CppVersion' A string representing the c++ version.
- 'SupportedPlatforms' Enum flag (int).
- 'OptimisationType' A string representing the optimisation type.
- 'FloatingPointType' The floating point optimisation type.

CVersion values : 

- "ANSI" or "C90"
- "C99"
- "C11"
- "C17"
- "C23"

CppVersion values : 

- "C++98"
- "C++03"
- "C++11"
- "C++14"
- "C++17"
- "C++20"
- "C++23"
- "C++26"

SupportedPlatforms values :

- Windows = 1
- MacOS   = 2
- Linux   = 3
- Android = 4
- IOS     = 5
- FreeBSD = 16
- OpenBSD = 17

OptimisationType values :

- "None"
- "Standard"
- "Agressive"
- "Fast"
- "MinSize"

FloatingPointType values :

- "Strict" Keep strict IEEE-754
- "Precise" Optimised but safe
- "Fast" Best optimisation level but less safe

Example : 
```lua
Platforms = {
    Windows = 1,
    Linux = 3,
    All = 0xFF
}

ReleaseTarget = {
    bAddDebugInfo = false,
    CVersion = "C17",
    CppVersion = "C++20",
    SupportedPlatforms = Platforms.Windows | Platforms.Linux,
    OptimisationType = "Fast",
    FloatingPointType = "Precise"
}
```

### Unit Rules Script {#unit-rules-script}

The unit table name is the unit name collapsed to "Unit"
The unit rules table require these fields :

- 'Modules' An array containing tables with module name as field name and a table as value for modules additional rules.
- 'SubUnits' An array containing tables with unit name as field name as value for sub-units additional rules.

Modules table additional rules fiels :

- None for now.

SubUnits table additional rules fiels :

- None for now.

Example :
```lua
ProjectUnit = {
    Modules = {
        ModuleTest = {
            -- Module additional data
        },
        ModuleTest2 = {
            -- Module additional data
        }
    },
    SubUnits = {
        PluginTest = {
            -- Unit additional data
        },
        PluginTest2 = {
            -- Unit additional data
        }
    }
}
```

### Module Rules Script {#module-rules-script}

Module rules table name is defined in the configuration file.
This table require :

- 'LinkingType' The module linking type.
- 'PublicIncludeDirectories' An array containing the public include directories.
- 'PrivateIncludeDirectories' An array containing the private include directories.
- 'PublicDependancies' An array containing the modules public dependancies.
- 'PrivateDependancies' An array containing the modules public dependancies.

Note : the module root is always a private include dir.


LinkingType values :

- "Static" Link as static library
- "Dynamic" Link as a dynamic library
- "DynamicLoading" Don't link, load dynamically

Exemple :
```lua
Module = {
    PublicIncludeDirectories = {"Public", "Interface"},
    PrivateIncludeDirectories = {"Public", "Interface"},

    PublicDependancies = {"TestModule", "TestModule2"},
    PrivateDependancies = {"TestModule3"},
    
    LinkingType = "Static"
}
```
