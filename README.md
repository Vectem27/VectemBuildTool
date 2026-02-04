# Vectem Build Tool

## Presentation

Vectem build tool is a customisable C++ module based build system.

## Use

Pattern command :

```bash
VectemBuildTool BuildConfig.lua --target-dir targetDir/ --project ProjectDir/ TargetName --dependancy-project DepProjectDir/
```

Simple project example :

```bash
VectemBuildTool BuildConfig.lua --target-dir Targets/ --project ExampleProject/ Debug
```

Project with engine example :

```bash
VectemBuildTool BuildConfig.lua --target-dir Engine/Targets/ --project Project/ Release --dependancy-project Engine/
```