-- Release Target Configuration
-- Optimized for performance without debug information

ReleaseTagetRules = TargetRules({
    bAddDebugInfo = false,
    CVersion = CVersions.C17,
    CppVersion = CppVersions.CPP20,
    SupportedPlatforms = Platforms.All,
    OptimisationType = OptimisationTypes.Fast,
    FloatingPointType = FloatingPointTypes.Fast
})
