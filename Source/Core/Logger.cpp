#include "Logger.hpp"

#include <chrono>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <type_traits>

namespace
{
    constexpr const char* ANSI_RESET = "\033[0m";

    LogLevel g_logLevel = LogLevel::Info;
    std::ofstream g_outputFile;
    std::mutex g_loggerMutex;

    const char* GetTerminalColor(LogLevel level) noexcept
    {
        switch (level)
        {
            case LogLevel::Critical:
                return "\033[95m";
            case LogLevel::Error:
                return "\033[31m";
            case LogLevel::Warning:
                return "\033[38;5;208m";
            case LogLevel::Info:
                return "\033[37m";
            case LogLevel::Debug:
                return "\033[90m";
            case LogLevel::Trace:
                return "\033[90m";
        }

        return ANSI_RESET;
    }

    std::tm GetLocalTime()
    {
        const auto now = std::chrono::system_clock::now();
        const std::time_t nowTime = std::chrono::system_clock::to_time_t(now);

        std::tm localTime{};
#ifdef _WIN32
        localtime_s(&localTime, &nowTime);
#else
        localtime_r(&nowTime, &localTime);
#endif

        return localTime;
    }

    std::string GetConsoleTimestamp()
    {
        const std::tm localTime = GetLocalTime();
        std::ostringstream stream;
        stream << std::put_time(&localTime, "%H:%M:%S");
        return stream.str();
    }

    std::string GetFileTimestamp()
    {
        const std::tm localTime = GetLocalTime();
        std::ostringstream stream;
        stream << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
        return stream.str();
    }

    std::string NormalizeLogLevelString(std::string_view value)
    {
        std::string normalized;
        normalized.reserve(value.size());

        for (const char character : value)
        {
            if (character >= 'A' && character <= 'Z')
            {
                normalized.push_back(static_cast<char>(character - 'A' + 'a'));
                continue;
            }

            normalized.push_back(character);
        }

        return normalized;
    }

    std::string FormatMessage(const char* format, std::va_list arguments)
    {
        std::va_list copy;
        va_copy(copy, arguments);
        const int requiredSize = std::vsnprintf(nullptr, 0, format, copy);
        va_end(copy);

        if (requiredSize < 0)
        {
            throw std::runtime_error("Unable to format log message");
        }

        std::string message(static_cast<size_t>(requiredSize), '\0');
        std::vsnprintf(message.data(), message.size() + 1, format, arguments);
        return message;
    }
}

std::strong_ordering operator<=>(LogLevel left, LogLevel right) noexcept
{
    using UnderlyingType = std::underlying_type_t<LogLevel>;

    const auto leftValue = static_cast<UnderlyingType>(left);
    const auto rightValue = static_cast<UnderlyingType>(right);

    if (leftValue < rightValue)
    {
        return std::strong_ordering::less;
    }

    if (leftValue > rightValue)
    {
        return std::strong_ordering::greater;
    }

    return std::strong_ordering::equal;
}

std::string_view ToString(LogLevel level) noexcept
{
    switch (level)
    {
        case LogLevel::Critical:
            return "Critical";
        case LogLevel::Error:
            return "Error";
        case LogLevel::Warning:
            return "Warning";
        case LogLevel::Info:
            return "Info";
        case LogLevel::Debug:
            return "Debug";
        case LogLevel::Trace:
            return "Trace";
    }

    return "Unknown";
}

std::optional<LogLevel> TryParseLogLevel(std::string_view value) noexcept
{
    const std::string normalized = NormalizeLogLevelString(value);

    if (normalized == "critical")
    {
        return LogLevel::Critical;
    }

    if (normalized == "error")
    {
        return LogLevel::Error;
    }

    if (normalized == "warning" || normalized == "warn")
    {
        return LogLevel::Warning;
    }

    if (normalized == "info")
    {
        return LogLevel::Info;
    }

    if (normalized == "debug")
    {
        return LogLevel::Debug;
    }

    if (normalized == "trace")
    {
        return LogLevel::Trace;
    }

    return std::nullopt;
}

LogLevel LogLevelFromString(std::string_view value)
{
    const auto parsedLevel = TryParseLogLevel(value);
    if (!parsedLevel.has_value())
    {
        throw std::invalid_argument("Unknown log level: " + std::string(value));
    }

    return *parsedLevel;
}

std::ostream& operator<<(std::ostream& output, LogLevel level)
{
    output << ToString(level);
    return output;
}

std::istream& operator>>(std::istream& input, LogLevel& level)
{
    std::string value;
    input >> value;

    const auto parsedLevel = TryParseLogLevel(value);
    if (!parsedLevel.has_value())
    {
        input.setstate(std::ios::failbit);
        return input;
    }

    level = *parsedLevel;
    return input;
}

void Logger::SetLevel(LogLevel level)
{
    std::scoped_lock lock(g_loggerMutex);
    g_logLevel = level;
}

LogLevel Logger::GetLevel()
{
    std::scoped_lock lock(g_loggerMutex);
    return g_logLevel;
}

void Logger::SetOutputFile(const std::optional<std::filesystem::path>& filePath)
{
    std::scoped_lock lock(g_loggerMutex);

    g_outputFile.close();
    if (!filePath.has_value())
    {
        return;
    }

    g_outputFile.open(*filePath, std::ios::out | std::ios::app);
    if (!g_outputFile.is_open())
    {
        throw std::runtime_error("Unable to open log file: " + filePath->string());
    }
}

void Logger::SetOutputFile(const std::filesystem::path& filePath)
{
    SetOutputFile(std::optional<std::filesystem::path>(filePath));
}

void Logger::ClearOutputFile()
{
    SetOutputFile(std::nullopt);
}

void Logger::Log(LogLevel level, const char* format, ...)
{
    std::va_list arguments;
    va_start(arguments, format);
    VLog(level, format, arguments);
    va_end(arguments);
}

void Logger::VLog(LogLevel level, const char* format, std::va_list arguments)
{
    const std::string message = FormatMessage(format, arguments);
    Write(level, message);
}

bool Logger::ShouldLog(LogLevel level)
{
    return g_logLevel >= level;
}

void Logger::Write(LogLevel level, const std::string& message)
{
    std::scoped_lock lock(g_loggerMutex);

    if (!ShouldLog(level))
    {
        return;
    }

    const std::string consoleTimestamp = GetConsoleTimestamp();
    const std::string fileTimestamp = GetFileTimestamp();

    std::cout << GetTerminalColor(level)
              << '[' << consoleTimestamp << "] "
              << '[' << level << "] " << message
              << ANSI_RESET << std::endl;

    if (g_outputFile.is_open())
    {
        g_outputFile << '[' << fileTimestamp << "] "
                     << '[' << level << "] " << message << std::endl;
        g_outputFile.flush();
    }
}