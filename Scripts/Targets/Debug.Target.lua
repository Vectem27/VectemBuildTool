-- Debug Target Configuration
-- Includes debug information for development

DebugTagetRules = TargetRules({
    bAddDebugInfo = true,
    CVersion = CVersions.C17,
    CppVersion = CppVersions.CPP20,
    SupportedPlatforms = Platforms.All,
    OptimisationType = OptimisationTypes.None,
    FloatingPointType = FloatingPointTypes.Precise
})
