ExeModuleTest_Rules = Module({
    PrivateDependencies = {"ModuleTest", "EngineModuleTest"},
    AdditionalMacro = { 
        { 
            Name = "_DEFAULT_SOURCE"
        },
        {
            Name = "TEST_MACRO_VAL",
            Value = '"StringValue"'
        }
    },
})