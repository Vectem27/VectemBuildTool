#pragma once

#include <compare>
#include <cstdarg>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

enum class LogLevel
{
    Critical,
    Error,
    Warning,
    Info,
    Debug,
    Trace
};

std::strong_ordering operator<=>(LogLevel left, LogLevel right) noexcept;

std::string_view ToString(LogLevel level) noexcept;
std::optional<LogLevel> TryParseLogLevel(std::string_view value) noexcept;
LogLevel LogLevelFromString(std::string_view value);

std::ostream& operator<<(std::ostream& output, LogLevel level);
std::istream& operator>>(std::istream& input, LogLevel& level);

using enum LogLevel;

class Logger
{
public:
    static void SetLevel(LogLevel level);
    static LogLevel GetLevel();

    static void SetOutputFile(const std::optional<std::filesystem::path>& filePath);
    static void SetOutputFile(const std::filesystem::path& filePath);
    static void ClearOutputFile();

    static void Log(LogLevel level, const char* format, ...);
private:
    static void VLog(LogLevel level, const char* format, std::va_list arguments);
    static bool ShouldLog(LogLevel level);
    static void Write(LogLevel level, const std::string& message);
};
