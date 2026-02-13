# Vectem Build Tool

## Overview

Vectem Build Tool is a customizable, module-based C++ build system designed to provide flexible project organization and fine-grained compilation control.

It is built around four core concepts:

- Targets  
- Units  
- Modules  
- Build Configuration  

All configuration files are written in Lua, allowing dynamic and programmable build logic.

# Getting Started

## Command Pattern

```xml
VectemBuildTool [--config <BuildConfigFile> --platform <PlatformName> --dependency-projects <ProjectRootDir> <UnitName> <UnitType> <Target>...] <ProjectRootDir> <UnitName> <UnitType> <Target>
```

Can have multiple --dependency-projects

### Example: Simple Project

```bash
VectemBuildTool Project/ Project ProjectUnit Debug
```

### Example: Project with Dependency

```bash
VectemBuildTool --config GlobalBuildConfig.lua --dependency-projects Engine/ Engine EnginUnit Release Project/ Project ProjectUnit Debug
```

# Project Structure

A project requires four types of configuration scripts:

1. Build Configuration Script  
2. Target Rules Script  
3. Unit Rules Script  
4. Module Rules Script  

All scripts are Lua files and are executed in the following order:

1. Build configuration script  
2. Target rules script  
3. Unit rules script  
4. Module rules script  

Unit rules are executed before module rules.

The system is highly customizable. You can modify folder structures, naming conventions, and rule behaviors.

# Core Concepts

## Modules

A module is a code container.

- Modules can depend on other modules.
- Modules may override compilation settings such as optimization level or language version.
- A module cannot be compiled alone; it must belong to a unit.

## Units

Units (or build units) contain modules.

A unit can be compiled as:
- An executable
- A static library
- A dynamic library

Units may contain sub-units depending on the build configuration.

Units can depend on other units.

External unit dependencies are specified in the build command and are compiled separately. Their configuration may differ from the main project.

Sub-units can optionally be built together with the main unit.

## Targets

A project may define multiple targets (e.g., Debug, Release).

Targets define:
- Compilation options
- Optimization level
- Debug information
- Platform support

Modules may override some target settings.

Sub-units always use the same target as their parent unit.

# Scripts

## Build Configuration Script

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
UnitsConfig
```

`UnitsConfig` defines unit types and their structure.

### Unit Type Fields

Each unit type must define:

- `ModulesDir` (array, cannot be empty)
- `ModuleRootName`
- `ModuleFileName`
- `ModuleClassName`
- `BuildDir`
- `SubUnits` (array, can be empty)

Supported macro:
- `${ModuleName}`

### SubUnits Fields

Each sub-unit entry must define:

- `Dir`
- `bRecursive`
- `UnitType`
- `UnitRootName`
- `UnitFileName`

Supported macro:
- `${UnitName}`

### Additional Fields for Root Units

- `TargetsDir`
- `TargetFileName`
- `TargetClassName`

Supported macro:
- `${TargetName}`

## Target Rules Script

A target rules script must define:

- `bAddDebugInfo`
- `CVersion`
- `CppVersion`
- `SupportedPlatforms`
- `OptimisationType`
- `FloatingPointType`

### Supported C Versions

- C90
- C99
- C11
- C17
- C23

### Supported C++ Versions

- C++98
- C++03
- C++11
- C++14
- C++17
- C++20
- C++23
- C++26

### Supported Platforms (Enum Flags)

```lua
Platforms = {
    Windows = 1,
    MacOS   = 2,
    Linux   = 4,
    Android = 8,
    IOS     = 16,
    FreeBSD = 32,
    OpenBSD = 64,
    All     = 0xFF
}
```

### Optimisation Types

- None
- Standard
- Aggressive
- Fast
- MinSize

### Floating Point Types

- Strict   (Strict IEEE-754 compliance)
- Precise  (Optimized but safe)
- Fast     (Maximum optimization, reduced precision guarantees)

## Unit Rules Script

The unit rules table name must be the unit name

Required fields:

- `Modules` A map containing the module name as key and a table containing additional data as value
- `SubUnits` A map containing the unit name as key and a table containing additional data as value

Example:

```lua
ProjectUnit = {
    Modules = {
        Core = {},
        Renderer = {}
    },
    SubUnits = {
        Editor = {}
    }
}
```

## Module Rules Script

A module rules script must define:

- `LinkingType`
- `PublicIncludeDirectories`
- `PrivateIncludeDirectories`
- `PublicDependencies`
- `PrivateDependencies`

Note:
The module root directory is always added as a private include directory.

### Linking Types

- Static
- Dynamic
- DynamicLoading

### Example

```lua
CoreRules = {
    PublicIncludeDirectories = { "Public", "Interface" },
    PrivateIncludeDirectories = { "Private" },

    PublicDependencies = { "Utils" },
    PrivateDependencies = { "InternalHelpers" },

    LinkingType = "Static"
}
```
