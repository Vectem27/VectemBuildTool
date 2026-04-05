#pragma once

#include <functional>
#include <optional>
#include <string>
#include <filesystem>
#include <vector>


class CompileFileChangeManager
{
    using fs_path = std::filesystem::path;
    static std::string DefaultHash(const std::string& str) { return std::to_string(std::hash<std::string>{}(str)); }

    struct FileStatus
    {
        fs_path path;
        std::string hash;
        std::filesystem::file_time_type timestamp;
    };
public:
    CompileFileChangeManager(std::function<std::string(const std::string&)> hashFunction = DefaultHash)
        : hashFunction(std::move(hashFunction)) {}

    virtual ~CompileFileChangeManager() = default;

    /**
     * @brief Check if a file needs to be rebuilt using its dependencies
     *
     * @param fileId The id for the file to check
     * @param dependencyFileIds The file dependency ids
     * @param cacheFiles The code file cache files
     * @return true if the file needs to be rebuilt
     */
    bool NeedsRebuild(const std::string& fileId, const std::vector<std::string>& dependencyFileIds, const std::vector<fs_path>& cacheFiles) const;

    /**
     * @brief Get the cache file path for a build output directory.
     *
     * @param buildOutputPath The build output directory.
     * @return The cache file path used to track file changes for that build output.
     */
    virtual fs_path GetCacheFilePath(const fs_path& buildOutputPath) const;

    /**
     * @brief Build the ordered list of cache files to inspect for a compilation.
     *
     * @param buildOutputPath The current build output.
     * @param dependencyBuildOutputs Other build outputs to inspect, typically from dependencies.
     * @return The cache files attached to all provided build outputs.
     */
    virtual std::vector<fs_path> GetCacheFiles(const fs_path& buildOutputPath, const std::vector<fs_path>& dependencyBuildOutputs) const;

    /**
     * @brief Add or update file informations into the cache file.
     *
     * @param fileId
     * @param file
     * @param cacheFilePath
     */
    virtual void CacheFile(const std::string& fileId, const fs_path& file, const fs_path& cacheFilePath);

protected:
    virtual FileStatus GetFileStatus(const fs_path& file) const;

    virtual std::optional<FileStatus> SearchCachedFileStatus(const std::string& fileId, const fs_path& cacheFilePath) const;

    std::string GetFileHash(fs_path file);
protected:
    std::function<std::string(const std::string&)> hashFunction;
};