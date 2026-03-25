#include "CompileCommandsExporter.h"

#include <fstream>
#include <mutex>
#include <unordered_map>

namespace fs = std::filesystem;

namespace
{
    struct CompileCommandEntry
    {
        fs::path workingDirectory;
        fs::path sourceFile;
        std::vector<std::string> arguments;
    };

    std::mutex g_compileCommandsMutex;
    std::unordered_map<std::string, std::vector<CompileCommandEntry>> g_entriesByBuildDirectory;

    std::string EscapeJsonString(const std::string& value)
    {
        std::string escaped;
        escaped.reserve(value.size());

        for (const char character : value)
        {
            switch (character)
            {
                case '\\':
                    escaped += "\\\\";
                    break;
                case '"':
                    escaped += "\\\"";
                    break;
                case '\n':
                    escaped += "\\n";
                    break;
                case '\r':
                    escaped += "\\r";
                    break;
                case '\t':
                    escaped += "\\t";
                    break;
                default:
                    escaped += character;
                    break;
            }
        }

        return escaped;
    }

    fs::path GetCompileCommandsPath(const fs::path& buildOutputPath)
    {
        return buildOutputPath / "compile_commands.json";
    }

    void WriteCompileCommandsFile(const fs::path& buildOutputPath, const std::vector<CompileCommandEntry>& entries)
    {
        fs::create_directories(buildOutputPath);

        std::ofstream outputFile(GetCompileCommandsPath(buildOutputPath), std::ios::out | std::ios::trunc);
        if (!outputFile.is_open())
        {
            throw std::runtime_error("Unable to open compile_commands.json in: " + buildOutputPath.string());
        }

        outputFile << "[\n";

        for (size_t index = 0; index < entries.size(); ++index)
        {
            const auto& entry = entries[index];

            outputFile << "  {\n";
            outputFile << "    \"directory\": \"" << EscapeJsonString(entry.workingDirectory.string()) << "\",\n";
            outputFile << "    \"file\": \"" << EscapeJsonString(entry.sourceFile.string()) << "\",\n";
            outputFile << "    \"arguments\": [";

            for (size_t argIndex = 0; argIndex < entry.arguments.size(); ++argIndex)
            {
                if (argIndex != 0)
                {
                    outputFile << ", ";
                }

                outputFile << "\"" << EscapeJsonString(entry.arguments[argIndex]) << "\"";
            }

            outputFile << "]\n";
            outputFile << "  }";

            if (index + 1 < entries.size())
            {
                outputFile << ",";
            }

            outputFile << "\n";
        }

        outputFile << "]\n";
    }
}

void CompileCommandsExporter::Reset(const fs::path& buildOutputPath)
{
    std::scoped_lock lock(g_compileCommandsMutex);

    const std::string key = fs::absolute(buildOutputPath).string();
    auto& entries = g_entriesByBuildDirectory[key];
    entries.clear();

    WriteCompileCommandsFile(fs::absolute(buildOutputPath), entries);
}

void CompileCommandsExporter::Append(
    const fs::path& buildOutputPath,
    const fs::path& workingDirectory,
    const fs::path& sourceFile,
    const std::vector<std::string>& arguments)
{
    std::scoped_lock lock(g_compileCommandsMutex);

    const fs::path absoluteBuildOutput = fs::absolute(buildOutputPath);
    const std::string key = absoluteBuildOutput.string();

    auto& entries = g_entriesByBuildDirectory[key];
    entries.push_back({
        .workingDirectory = fs::absolute(workingDirectory),
        .sourceFile = fs::absolute(sourceFile),
        .arguments = arguments,
    });

    WriteCompileCommandsFile(absoluteBuildOutput, entries);
}