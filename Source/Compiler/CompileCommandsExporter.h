#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace CompileCommandsExporter
{
    void Reset(const std::filesystem::path& buildOutputPath);

    void Append(
        const std::filesystem::path& buildOutputPath,
        const std::filesystem::path& workingDirectory,
        const std::filesystem::path& sourceFile,
        const std::vector<std::string>& arguments
    );
}