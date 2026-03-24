
#include "TargetRulesReader.h"
#include "Target/ITargetRulesReader.h"

#include <sol/sol.hpp>
#include <stdexcept>

CVersion TargetRulesReader::StringToCVersion(const std::string& value) const
{
    if (value == "C90") return CVersion::C90;
    if (value == "C99") return CVersion::C99;
    if (value == "C11") return CVersion::C11;
    if (value == "C17") return CVersion::C17;
    if (value == "C23") return CVersion::C23;
    throw std::invalid_argument("Invalid C version: " + value);
}

CppVersion TargetRulesReader::StringToCppVersion(const std::string& value) const
{
    if (value == "CPP98") return CppVersion::CPP98;
    if (value == "CPP03") return CppVersion::CPP03;
    if (value == "CPP11") return CppVersion::CPP11;
    if (value == "CPP14") return CppVersion::CPP14;
    if (value == "CPP17") return CppVersion::CPP17;
    if (value == "CPP20") return CppVersion::CPP20;
    if (value == "CPP23") return CppVersion::CPP23;
    if (value == "CPP26") return CppVersion::CPP26;
    throw std::invalid_argument("Invalid C++ version: " + value);
}

CompilationOptimisation TargetRulesReader::StringToOptimisationType(const std::string& value) const
{
    if (value == "None") return CompilationOptimisation::NONE;
    if (value == "Standard") return CompilationOptimisation::STANDARD;
    if (value == "Aggressive") return CompilationOptimisation::AGGRESSIVE;
    if (value == "Fast") return CompilationOptimisation::FAST;
    if (value == "MinSize") return CompilationOptimisation::MIN_SIZE;
    throw std::invalid_argument("Invalid optimization type: " + value);
}

FloatingPointModel TargetRulesReader::StringToFloatingPointType(const std::string& value) const
{
    if (value == "Strict") return FloatingPointModel::STRICT;
    if (value == "Precise") return FloatingPointModel::PRECISE;
    if (value == "Fast") return FloatingPointModel::FAST;
    throw std::invalid_argument("Invalid floating point type: " + value);
}

TargetRules TargetRulesReader::ReadRules(const std::string& targetName, const std::string& targetRulesField) const
{
    TargetRules rules;

    rules.name = targetName;

    try
    {
        sol::optional<sol::table> unitRulesField = lua[targetRulesField].get<sol::optional<sol::table>>();
        if (!unitRulesField)
            throw TargetRulesReaderException("Target rules field is missing for : '" + targetName + "'.");

        sol::table unitRules = unitRulesField.value();

        // Read bAddDebugInfo
        sol::optional<bool> bAddDebugInfo = unitRules["bAddDebugInfo"].get<sol::optional<bool>>();
        if (bAddDebugInfo)
            rules.bAddDebugInfo = bAddDebugInfo.value();

        // Read CVersion
        sol::optional<std::string> cVersion = unitRules["CVersion"].get<sol::optional<std::string>>();
        if (cVersion)
            rules.cVersion = StringToCVersion(cVersion.value());

        // Read CppVersion
        sol::optional<std::string> cppVersion = unitRules["CppVersion"].get<sol::optional<std::string>>();
        if (cppVersion)
            rules.cppVersion = StringToCppVersion(cppVersion.value());

        // Read SupportedPlatforms
        sol::optional<int> supportedPlatforms = unitRules["SupportedPlatforms"].get<sol::optional<int>>();
        if (supportedPlatforms)
            rules.supportedPlatforms = supportedPlatforms.value();

        // Read OptimisationType
        sol::optional<std::string> optimisationType = unitRules["OptimisationType"].get<sol::optional<std::string>>();
        if (optimisationType)
            rules.optimisationType = StringToOptimisationType(optimisationType.value());

        // Read FloatingPointType
        sol::optional<std::string> floatingPointType = unitRules["FloatingPointType"].get<sol::optional<std::string>>();
        if (floatingPointType)
            rules.floatingPointType = StringToFloatingPointType(floatingPointType.value());
    }
    catch (const std::exception& e)
    {
        throw TargetRulesReaderException(e.what());
    }
    catch (...)
    {
        throw TargetRulesReaderException("Unkown exception");
    }

    return rules;
}
