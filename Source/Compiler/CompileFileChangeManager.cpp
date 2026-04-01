
#include "CompileFileChangeManager.h"
#include <chrono>
#include <fstream>
#include <sstream>
#include <unordered_map>

bool CompileFileChangeManager::NeedsRebuild(const std::string& fileId, const std::vector<std::string>& dependencyFileIds,
                                            const std::vector<fs_path>& cacheFiles) const
{
    // Try to find cached status for the target file in provided cache files
    std::optional<FileStatus> cachedTarget;
    for (const auto& cache : cacheFiles)
    {
        cachedTarget = SearchCachedFileStatus(fileId, cache);
        if (cachedTarget.has_value())
            break;
    }

    // If no cached entry, we must rebuild
    if (!cachedTarget.has_value())
        return true;

    // Get current status of the file referenced by the cache
    FileStatus currentTarget = GetFileStatus(cachedTarget->path);
    if (currentTarget.hash != cachedTarget->hash)
        return true;

    // Check dependencies
    for (const auto& depId : dependencyFileIds)
    {
        std::optional<FileStatus> cachedDep;
        for (const auto& cache : cacheFiles)
        {
            cachedDep = SearchCachedFileStatus(depId, cache);
            if (cachedDep.has_value())
                break;
        }

        // missing dependency cache -> rebuild
        if (!cachedDep.has_value())
            return true;

        FileStatus currentDep = GetFileStatus(cachedDep->path);
        if (currentDep.hash != cachedDep->hash)
            return true;
    }

    return false;
}

void CompileFileChangeManager::CacheFile(const std::string& fileId, const fs_path& file, const fs_path& cacheFilePath)
{
    // Load existing cache entries (simple key->line map)
    std::unordered_map<std::string, std::string> entries;
    if (std::filesystem::exists(cacheFilePath))
    {
        std::ifstream in(cacheFilePath);
        std::string line;
        while (std::getline(in, line))
        {
            auto sep = line.find('|');
            if (sep == std::string::npos)
                continue;
            std::string key = line.substr(0, sep);
            entries[key] = line;
        }
    }

    // Prepare new entry: fileId|path|hash|timestamp
    FileStatus status = GetFileStatus(file);
    using seconds = std::chrono::seconds;
    auto tp = status.timestamp.time_since_epoch();
    auto secs = std::chrono::duration_cast<seconds>(tp).count();
    std::ostringstream oss;
    oss << fileId << '|' << status.path.string() << '|' << status.hash << '|' << secs;
    entries[fileId] = oss.str();

    // Write back
    std::ofstream out(cacheFilePath, std::ios::trunc);
    for (const auto& kv : entries)
    {
        out << kv.second << '\n';
    }
}

CompileFileChangeManager::FileStatus CompileFileChangeManager::GetFileStatus(const fs_path& file) const
{
    FileStatus s;
    s.path = file;
    try
    {
        if (std::filesystem::exists(file) && std::filesystem::is_regular_file(file))
        {
            s.timestamp = std::filesystem::last_write_time(file);
            // need non-const GetFileHash which may read the file
            s.hash = const_cast<CompileFileChangeManager*>(this)->GetFileHash(file);
        }
        else
        {
            s.timestamp = std::filesystem::file_time_type::min();
            s.hash.clear();
        }
    }
    catch (const std::filesystem::filesystem_error&)
    {
        s.timestamp = std::filesystem::file_time_type::min();
        s.hash.clear();
    }
    return s;
}

std::optional<CompileFileChangeManager::FileStatus> CompileFileChangeManager::SearchCachedFileStatus(
    const std::string& fileId, const fs_path& cacheFilePath) const
{
    if (!std::filesystem::exists(cacheFilePath))
        return std::nullopt;

    std::ifstream in(cacheFilePath);
    if (!in)
        return std::nullopt;

    std::string line;
    while (std::getline(in, line))
    {
        // expected format: key|path|hash|timestamp
        std::vector<std::string> parts;
        std::string cur;
        std::istringstream iss(line);
        while (std::getline(iss, cur, '|'))
            parts.push_back(cur);
        if (parts.size() < 4)
            continue;
        if (parts[0] != fileId)
            continue;

        FileStatus s;
        s.path = parts[1];
        s.hash = parts[2];
        try
        {
            unsigned long long secs = std::stoull(parts[3]);
            using seconds = std::chrono::seconds;
            s.timestamp = std::filesystem::file_time_type(seconds(secs));
        }
        catch (...)
        {
            s.timestamp = std::filesystem::file_time_type::min();
        }

        return s;
    }

    return std::nullopt;
}

std::string CompileFileChangeManager::GetFileHash(fs_path file)
{
    std::string content;
    std::ifstream in(file, std::ios::binary);
    if (!in)
        return {};
    std::ostringstream ss;
    ss << in.rdbuf();
    content = ss.str();
    try
    {
        return hashFunction(content);
    }
    catch (...)
    {
        return {};
    }
}
