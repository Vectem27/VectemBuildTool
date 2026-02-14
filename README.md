# Vectem Build Tool

## Overview

Vectem Build Tool is a customizable, module-based C++ build system designed to provide flexible project organization and fine-grained compilation control.

Document sections :

- [Getting Started](#getting-started)
- [Project Structure](#project-structure)
- [Scripts](#scripts)

It is built around four core concepts:

- [Targets](#targets)
- [Units](#units)
- [Modules](#modules)
- [Build Configuration](#build-configuration)

All configuration files are written in Lua, allowing dynamic and programmable build logic.

## Getting Started

### Command Pattern

```xml
VectemBuildTool [--config <BuildConfigFile> --platform <PlatformName> --dependency-projects <ProjectRootDir> <UnitName> <UnitType> <Target>...] <ProjectRootDir> <UnitName> <UnitType> <Target>
```

Can have multiple --dependency-projects

#### Example: Simple Project

```bash
VectemBuildTool Project/ Project ProjectUnit Debug
```

#### Example: Project with Dependency

```bash
VectemBuildTool --config GlobalBuildConfig.lua --dependency-projects Engine/ Engine EnginUnit Release Project/ Project ProjectUnit Debug
```

---

## Project Structure

A project requires four types of configuration scripts:

1. Build Configuration Script  
2. Target Rules Script  
3. Unit Rules Script  
4. Module Rules Script  

All scripts are Lua files and are executed in the following order for units and modules:

1. Build configuration script  
2. Unit rules script  
3. Module rules script  

And for target:

1. Build configuration script  
2. Target rules script

Unit rules are executed before module rules.

The system is highly customizable. You can modify folder structures, naming conventions, and rule behaviors.

### Core Concepts

#### Modules

A module is a code container.

- Modules can depend on other modules.
- Modules may override compilation settings such as optimization level or language version.
- A module cannot be compiled alone; it must belong to a unit.

#### Units

Units (or build units) contain modules.

A unit can be compiled as:
- An executable
- A static library
- A dynamic library

Units may contain sub-units depending on the build configuration.

Units can depend on other units.

External unit dependencies are specified in the build command and are compiled separately. Their configuration may differ from the main project.

Sub-units can optionally be built together with the main unit.

#### Targets

A project may define multiple targets (e.g., Debug, Release).

Targets define:
- Compilation options
- Optimization level
- Debug information
- Platform support

Modules may override some target settings.

Sub-units always use the same target as their parent unit.

#### Build Configuration

The build configuration define units structure and some units rules. 
It defines possible units types, them folder structure, them sub-units and some defaults rules.

---

## Scripts

See 

- [Build Configuration Script](#config-script)
- [Target Rules Script](#target-script)
- [Unit Rules Script](#unit-script)
- [Module Rules Script](#module-script)

**Note** : You can add every fields you want

### Build Configuration Script {#config-script}

By default, the build system uses:

```
BuildConfig.lua
```

located at the project root.

You can override it using:

```
--config
```

The script must define a table named:

```
BuildConfig
```

`BuildConfig` defines unit types and their structure.

##### Unit Type Fields

Each unit type must define:

- `UnitFileName` The unit file name.
- `UnitClassName` The unit rules table name.
- `ModulesDirs` (array, cannot be empty)
- `ModuleRootName` The module root folder name.
- `ModuleFileName` The module rules script file name.
- `ModuleClassName` The modules rules table name.
- `BuildDir` The output build directory.
- `SubUnits` (array, can be empty)

Supported macro:

- `${ModuleName}` (For `ModuleRootName`, `ModuleFileName` and `ModuleClassName`.)
- `${UnitName}` (For `UnitFileName`.)

##### SubUnits Fields

Each sub-unit entry must define:

- `Dir` The directory containing these sub-units.
- `bRecursive` Boolean, recursive path search.
- `UnitType` The unit type.
- `UnitRootName` The unit root folder name.

Supported macro:
- `${UnitName}` (For `UnitRootName` and `UnitFileName`)

##### Additional Fields for Root Units

- `TargetsDir` The target rules directory.
- `TargetFileName` The target rules script file name.
- `TargetClassName` The target rules table name.

Supported macro:
- `${TargetName}` (For `TargetFileName` and `TargetClassName`.)

#### Example

```lua 
BuildConfig = { 
    Program = { 
        UnitFileName = "${UnitName}.Program.lua",

        ModulesDir = { "Modules" }, 
        ModuleRootName = "${ModuleName}Module", 
        ModuleFileName = "${ModuleName}.Module.lua", 
        ModuleClassName = "${ModuleName}Rules",
        
        TargetsDir = "Targets", 
        TargetFileName = "${TargetName}.Target.lua", 
        TargetClassName = "${TargetName}TagetRules", 

        BuildDir = "Build", 

        SubUnits = { 
            { 
                Dir = "Plugins", 
                UnitType = "Plugin", 
                UnitRootName = "${UnitName}", 
                bRecursive=true 
            } 
        } 
    },
    Plugin = { 
        UnitFileName = "${UnitName}.Plugin.lua",

        ModulesDir = { "Modules" }, 
        ModuleRootName = "${ModuleName}", 
        ModuleFileName = "${ModuleName.Build.lua}", 
        ModuleClassName = "${ModuleName}Rules", 

        BuildDir = "Build", 

        SubUnits = {} 
    } 
}
```

---

### Target Rules Script {#target-script}

A target rules script must define:

- `bAddDebugInfo`
- `CVersion`
- `CppVersion`
- `SupportedPlatforms`
- `OptimisationType`
- `FloatingPointType`

#### Supported C Versions

- C90
- C99
- C11
- C17
- C23

#### Supported C++ Versions
- C++98
- C++03
- C++11
- C++14
- C++17
- C++20
- C++23
- C++26

#### Supported Platforms (Enum Flags) {#platforms}

Lua code :

```lua
Platforms = {
    Windows = 1,
    MacOS   = 2,
    Linux   = 4,
    Android = 8,
    IOS     = 16,
    FreeBSD = 32,
    OpenBSD = 64,
    NetBSD = 128,
    All     = 0xFF
}
```

#### Optimisation Types

- None
- Standard
- Aggressive
- Fast
- MinSize

#### Floating Point Types

- Strict   (Strict IEEE-754 compliance)
- Precise  (Optimized but safe)
- Fast     (Maximum optimization, reduced precision guarantees)

#### Example

```lua
ReleaseTarget = { 
    bAddDebugInfo = false, 
    CVersion = "C17", 
    CppVersion = "C++20", 
    SupportedPlatforms = Platforms.Windows | Platforms.Linux, 
    OptimisationType = "Fast", 
    FloatingPointType = "Precise" 
}
```

---

### Unit Rules Script {#unit-script}

The unit rules table name must be the unit name

Required fields:

- `CompilationType`
- `Modules` A map containing the module name as key and a table containing additional data as value
- `SubUnits` A map containing the unit name as key and a table containing additional data as value

#### Compilation types

- Executable : Create an executable for the unit.
- Library : Don't create an executable. Only libraries build.

#### Additional data

Possible fields for both Modules and SubUnits:

- `Platforms` Compile only on some platforms. See [Platforms](#platforms)

#### Example

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

---

### Module Rules Script {#module-script}

A module rules script must define:

- `PublicIncludeDirectories`
- `PrivateIncludeDirectories`
- `PublicModuleDependencies`
- `PrivateModuleDependencies`
- `CodeDir` The code directory

Note:
The module root directory is always added as a private include directory.

#### Example

```lua
ModuleRules = {
    PublicIncludeDirectories = { "Public", "Interface" },
    PrivateIncludeDirectories = { "Private" },

    PublicDependencies = { "Utils" },
    PrivateDependencies = { "InternalHelpers" },

    LinkingType = "Static"
}
```

---