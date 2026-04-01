-- Shipping Target Configuration
-- Maximum optimization for production builds

Shipping_Rules = TargetRules({
    bAddDebugInfo = false,
    CVersion = CVersions.C17,
    CppVersion = CppVersions.CPP20,
    SupportedPlatforms = Platforms.Windows | Platforms.Linux | Platforms.MacOS,
    OptimisationType = OptimisationTypes.Aggressive,
    FloatingPointType = FloatingPointTypes.Fast
})
