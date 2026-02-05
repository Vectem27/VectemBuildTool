cmake_minimum_required(VERSION 3.20)

if(LINUX)
    set(CMAKE_SYSTEM_NAME Linux)

    set(TOOLCHAIN_ROOT ${CMAKE_SOURCE_DIR}/bin/Toolchains/linux)
    set(CMAKE_SYSROOT ${TOOLCHAIN_ROOT}/sysroot)

    set(CMAKE_CXX_COMPILER ${CMAKE_SYSROOT}/clang/bin/clang++)
    set(CMAKE_C_COMPILER ${CMAKE_SYSROOT}/clang/bin/clang)

    set(CMAKE_C_FLAGS_INIT   "--sysroot=${CMAKE_SYSROOT}")
    set(CMAKE_CXX_FLAGS_INIT "--sysroot=${CMAKE_SYSROOT} -stdlib=libc++")

    set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=lld")

    set(CMAKE_LINKER ${CMAKE_SYSROOT}/clang/bin/ld.lld)

    set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
endif()

project(VectemBuildTool
    VERSION 0.1.0
    LANGUAGES C CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

add_subdirectory(Source)

# Scripts
set(SCRIPT_DIR ${CMAKE_SOURCE_DIR}/Scripts)
if (EXISTS ${SCRIPT_DIR})
    file(COPY ${SCRIPT_DIR}
        DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()


# Lua

set(LIBS_DIR ${CMAKE_SOURCE_DIR}/ThirdParty)
set(LUA_DIR ${LIBS_DIR}/lua-5.4.8)

if(WIN32)
    set(LUA_LIB ${LUA_DIR}/lib/lua.lib)
    if(NOT EXISTS ${LUA_LIB})
        message(FATAL_ERROR "Lua library not found: ${LUA_LIB}")
    endif()
else()
    set(LUA_LIB ${LUA_DIR}/lib/liblua.a)
    if(NOT EXISTS ${LUA_LIB})
        message(FATAL_ERROR "Lua library not found: ${LUA_LIB}")
    endif()
endif()

target_link_libraries(VectemBuildTool PRIVATE ${LUA_LIB})
target_include_directories(VectemBuildTool SYSTEM PRIVATE ${LUA_DIR}/src)


# Sol2 (Include only)

set(SOL2_DIR ${LIBS_DIR}/sol2/)
set(SOL2_INC_DIR ${SOL2_DIR}/include)
target_include_directories(VectemBuildTool SYSTEM PRIVATE ${SOL2_INC_DIR})


# Cli11 (Include only)

set(CLI11_DIR ${LIBS_DIR}/CLI11/)
set(CLI11_INC_DIR ${CLI11_DIR})
target_include_directories(VectemBuildTool SYSTEM PRIVATE ${CLI11_INC_DIR})