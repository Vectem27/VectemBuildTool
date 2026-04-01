-- Test Target Configuration
-- All fields are optional and will use default values from BuildConfig if not specified

TestTagetRules = TargetRules({
    bAddDebugInfo = true,
    CVersion = CVersions.C17,
    CppVersion = CppVersions.CPP20,
    SupportedPlatforms = Platforms.Windows | Platforms.Linux | Platforms.MacOS,
    OptimisationType = OptimisationTypes.Standard,
    FloatingPointType = FloatingPointTypes.Precise
})