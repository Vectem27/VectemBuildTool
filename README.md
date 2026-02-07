# Vectem Build Tool

## Presentation

Vectem build tool is a customisable C++ module based build system.

## Use

Pattern command :

```bash
VectemBuildTool --config BuildConfig.lua --project ProjectDir/ TargetName --dependancy-project DepProjectDir/
```

Simple project example :

```bash
VectemBuildTool --config BuildConfig.lua --project ExampleProject/ --target Debug
```

Project with engine example :

```bash
VectemBuildTool --config BuildConfig.lua --project Project/ --target Release --dependancy-projects Engine/ --dependancy-targets Release
```