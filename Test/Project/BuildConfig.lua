---------------
-- Functions --
---------------

function EnumFlag(names)
    local e = {}
    local all = 0

    for i, name in ipairs(names) do
        local value = 1 << (i - 1)
        e[name] = value
        all = all | value
    end

    e.All = all

    return setmetatable(e, {
        __newindex = function()
            error("Enum is read-only")
        end
    })
end

function Enum(names)
    local e = {}

    for i, name in ipairs(names) do
        local value = i
        e[name] = value
    end

    return setmetatable(e, {
        __newindex = function()
            error("Enum is read-only")
        end
    })
end

local function RuleSet(defaults)
    return setmetatable(defaults, {
        __newindex = function()
            error("Rule set base is read-only")
        end
    })
end

local function UnitsConfigSet(defaults)
    return setmetatable(defaults, {
        __newindex = function()
            error("Units config set is read-only")
        end
    })
end



---------------
-- Variables --
---------------

local DefaultCppVersion = 20

Platforms = EnumFlag({
    "Windows",
    "Linux",
    "MacOS"
})


UnitCompilationTypes = Enum({
    "Executable",
    "Library",
})

------------
-- Config --
------------

-- Units config

UnitsConfig = UnitsConfigSet({
    UnitFileName = "${UnitName}.Unit.lua",

    Program = {
        ModulesDir      = { "Modules" },
        ModuleRootName  = "${ModuleName}",
        ModuleFileName  = "${ModuleName.Build.lua}",
        ModuleClassName = "${ModuleName}Rules", -- Not implemented

        BuildDir = "Build",

        TargetsDir = "Targets", -- TODO: check functionalities

        SubUnits = {
            {
                Dir = "Plugins",
                UnitType = "Plugin",
                UnitRootName = "${UnitName}",
                bRecursive=true
            }
        }
        -- TODO: Add regex for lua filenames e.g. for module : '{ModuleName}.Module.lua'
    },
    Engine = {
        ModulesDir      = { "Modules" }, -- Regex
        ModuleRootName  = "${ModuleName}",
        ModuleFileName  = "${ModuleName.Build.lua}",
        ModuleClassName = "${ModuleName}Rules",
        BuildDir = "Build",
        TargetsDir = "Targets",
        SubUnitsDir = {
            {Dir = "Plugins", UnitType = {"Plugin"}, bRecursive=true}
        }
    },
    Plugin = {
        ModulesDir      = { "Modules" }, -- Regex
        ModuleRootName  = "${ModuleName}",
        ModuleFileName  = "${ModuleName.Build.lua}",
        ModuleClassName = "${ModuleName}Rules",
        BuildDir = "Build",
        SubUnitsDir = {}
    }
})

-- Target rules base

local TargetDefaultRules = RuleSet({
    SupportedPlatforms = "All",
    OverrideUnitCompilationType = {
        -- e.g. Plugin = UnitCompilationTypes.Library
    }
})

function TargetRules(default)
    return setmetatable(defaults, {
        __index = TargetDefaultRules
    })
end

-- Module rules base

local ModuleDefaultRules = RuleSet({
    CppVersion = DefaultCppVersion,
})

function ModuleRules(defaults)
    return setmetatable(defaults, {
        __index = ModuleDefaultRules
    })
end

-- Units rules base

local ProgramDefaultRules = RuleSet({
    UnitType = "Program",
    CppVersion = DefaultCppVersion,
    UnitCompilationType = UnitCompilationTypes.Executable,
    Modules = {}
})

local EngineDefaultRules = RuleSet({
    UnitType = "Engine",
    CppVersion = DefaultCppVersion,
    UnitCompilationType = UnitCompilationTypes.StaticLibrary,
    Modules = {}
})

local PluginDefaultRules = RuleSet({
    UnitType = "Plugin",
    CppVersion = DefaultCppVersion,
    UnitCompilationType = UnitCompilationTypes.DynamicLibrary,
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
