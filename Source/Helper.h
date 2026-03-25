#pragma once

#include <filesystem>

static std::filesystem::path GetProgramDir() {
#if defined(_WIN32)
    #include <windows.h>
    char buf[MAX_PATH];
    GetModuleFileNameA(nullptr, buf, MAX_PATH);
    return std::filesystem::path(buf).parent_path();
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
    char buf[1024];
    uint32_t size = sizeof(buf);
    _NSGetExecutablePath(buf, &size);
    return std::filesystem::canonical(buf).parent_path();
#else
    return std::filesystem::read_symlink("/proc/self/exe").parent_path();
#endif
}

static inline std::filesystem::path GetScriptDir()
{
    return GetProgramDir() / "Scripts";
}