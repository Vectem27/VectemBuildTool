---------------
-- Functions --
---------------

function EnumFlag(names)
    local values = {}
    local all = 0

    for i, name in ipairs(names) do
        assert(values[name] == nil, "Duplicate enum value: " .. name)

        local value = 1 << (i - 1)
        values[name] = value
        all = all | value
    end

    values.All = all

    return setmetatable({}, {
        __index = values,
        __newindex = function()
            error("Enum is read-only", 2)
        end,
        __pairs = function()
            return pairs(values)
        end
    })
end

function Enum(names)
    assert(type(names) == "table", "Enum expects a table")

    local values = {}

    for i, name in ipairs(names) do
        assert(values[name] == nil, "Duplicate enum value: " .. name)
        values[name] = name
    end

    return setmetatable({}, {
        __index = function(_, key)
            local v = values[key]
            if v == nil then
                error("Invalid enum key: " .. tostring(key), 2)
            end
            return v
        end,

        __newindex = function()
            error("Enum is read-only", 2)
        end,

        __pairs = function()
            return pairs(values)
        end
    })
end


local function RuleSet(defaults)
    assert(type(defaults) == "table", "RuleSet expects a table")

    return setmetatable(defaults, {
        __newindex = function(table, key, _)
            error("Unknown key '" .. key .. " from rule set " .. table)
        end
    })
end

local function BuildConfigSet(defaults)
    assert(type(defaults) == "table", "BuildConfigSet expects a table")

    return setmetatable(defaults, {
        __newindex = function()
            error("Units config set is read-only")
        end
    })
end

local function Set(defaults)
    assert(type(defaults) == "table", "Set expects a table")

    return setmetatable(defaults, {
        __newindex = function(table, key, _)
            error("Unknown key '" .. key .. " from set " .. table)
        end
    })
end







---------------
-- Variables --
---------------

-- Platform definitions (Enum Flags)
Platforms = EnumFlag({
    "Windows",
    "MacOS",
    "Linux",
    "Android",
    "IOS",
    "FreeBSD",
    "OpenBSD",
    "NetBSD"
})

-- C language versions
CVersions = Enum({
    "C90",
    "C99",
    "C11",
    "C17",
    "C23"
})

-- C++ language versions
CppVersions = Enum({
    "CPP98",
    "CPP03",
    "CPP11",
    "CPP14",
    "CPP17",
    "CPP20",
    "CPP23",
    "CPP26"
})

-- Optimization types
OptimisationTypes = Enum({
    "None",
    "Standard",
    "Aggressive",
    "Fast",
    "MinSize"
})

-- Floating point model types
FloatingPointTypes = Enum({
    "Strict",
    "Precise",
    "Fast"
})

-- Unit compilation types
UnitCompilationTypes = Enum({
    "Executable",
    "Library",
})

-- Linking types
LinkingTypes = Enum({
    "Static",
    "Dynamic",
    "DynamicLoading"
})




------------
-- Config --
------------

-- Units config

BuildConfig = BuildConfigSet({
    Program = Set({
        UnitFileName = "${UnitName}.Unit.lua",
        UnitClassName = "${UnitName}UnitRules",

        ModulesDir      = { "Modules" },
        ModuleRootName  = "${ModuleName}Module",
        ModuleFileName  = "${ModuleName}.Module.lua",
        ModuleClassName = "${ModuleName}ModuleRules",

        TargetsDir = "Targets",
        TargetFileName  = "${TargetName}.Target.lua",
        TargetClassName = "${TargetName}TagetRules",

        BuildDir = "Build",

        SubUnits = {
            {
                Dir = "Plugins",
                UnitType = "Plugin",
                UnitRootName = "${UnitName}",
                bRecursive=true
            }
        }
    }),

    Engine = Set({
        UnitFileName = "${UnitName}.Unit.lua",
        UnitClassName = "${UnitName}UnitRules",
        
        ModulesDir      = { "Modules" },
        ModuleRootName  = "${ModuleName}",
        ModuleFileName  = "${ModuleName.Build.lua}",
        ModuleClassName = "${ModuleName}ModuleRules",

        TargetsDir = "Targets",
        TargetFileName  = "${TargetName}.Target.lua",
        TargetClassName = "${TargetName}TagetRules",

        BuildDir = "Build",

        SubUnits = {
            {
                Dir = "Plugins", 
                UnitType = "Plugin", 
                UnitRootName = "${UnitName}",
                bRecursive=true
            }
        }
    }),

    Plugin = Set({
        UnitFileName = "${UnitName}.Plugin.lua",
        UnitClassName = "${UnitName}UnitRules",

        ModulesDir      = { "Modules" },
        ModuleRootName  = "${ModuleName}",
        ModuleFileName  = "${ModuleName.Build.lua}",
        ModuleClassName = "${ModuleName}ModuleRules",
        BuildDir = "Build",
        SubUnits = {}
    })
})








-- Target rules base

local TargetDefaultRules = RuleSet({
    bAddDebugInfo = false,
    CVersion = CVersions.C17,
    CppVersion = CppVersions.CPP20,
    SupportedPlatforms = Platforms.All,
    OptimisationType = OptimisationTypes.Standard,
    FloatingPointType = FloatingPointTypes.Precise
    --OverrideUnitCompilationType = {
        -- e.g. Plugin = UnitCompilationTypes.Library
    --}
})

function TargetRules(default)
    return setmetatable(default, {
        __index = TargetDefaultRules
    })
end







-- Module rules base

local ModuleDefaultRules = RuleSet({
    CppVersion = CppVersions.CPP20,
    SupportedPlatforms = Platforms.All
})

function ModuleRules(defaults)
    return setmetatable(defaults, {
        __index = ModuleDefaultRules
    })
end








-- Units rules base

local ProgramDefaultRules = RuleSet({
    UnitCompilationType = UnitCompilationTypes.Executable,
    Modules = {}
})

local EngineDefaultRules = RuleSet({
    UnitCompilationType = UnitCompilationTypes.Library,
    Modules = {}
})

local PluginDefaultRules = RuleSet({
    UnitCompilationType = UnitCompilationTypes.Library,
    Modules = {}
})




-- Units creation functions

local function MakeUnit(defaults, rules)
    return setmetatable(defaults, {
        __index = rules
    })
end

EngineRules  = function(d) return MakeUnit(d, EngineDefaultRules) end
ProgramRules = function(d) return MakeUnit(d, ProgramDefaultRules) end
PluginRules  = function(d) return MakeUnit(d, PluginDefaultRules) end


-- Module base

local DefaultModuleRules = {
    PublicIncludeDirectories = {"Public"},
    PrivateIncludeDirectories = {"Private"},

    PublicDependencies = {},
    PrivateDependencies = {},

    CodeDir = "Source"
}

function Module(defaults)
    assert(type(defaults) == "table", "Module expects a table")

    return setmetatable(defaults, {
        __index = DefaultModuleRules;
        __newindex = function(table, key, _)
            error("Unknown key '" .. key .. " from module " .. table)
        end
    })
end
