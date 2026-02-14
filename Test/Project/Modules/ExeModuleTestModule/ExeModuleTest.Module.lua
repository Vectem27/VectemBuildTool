ExeModuleTestModuleRules = {
    PublicIncludeDirectories = {"Public", "Interface"},
    PrivateIncludeDirectories = {"Private"},

    PublicDependencies = {"ModuleTest"},
    PrivateDependencies = {},

    LinkingType = LinkingTypes.Static,
    
    CodeDir = "Source"
}