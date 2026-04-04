ExeModuleTest_Rules = Module({
    PrivateDependencies = {"ModuleTest", "EngineModuleTest"},
    AdditionalMacro = { 
        { 
            Name = "_DEFAULT_SOURCE"
        }
    },
})