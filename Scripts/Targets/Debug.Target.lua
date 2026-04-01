-- Debug Target Configuration
-- Includes debug information for development

Debug_Rules = TargetRules({
    bAddDebugInfo = true,
    CVersion = CVersions.C17,
    CppVersion = CppVersions.CPP20,
    SupportedPlatforms = Platforms.All,
    OptimisationType = OptimisationTypes.None,
    FloatingPointType = FloatingPointTypes.Precise
})
