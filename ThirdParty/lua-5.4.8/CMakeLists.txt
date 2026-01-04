cmake_minimum_required(VERSION 3.20)
project(lua LANGUAGES C)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

set(LUA_CORE
    src/lapi.c src/lcode.c src/lctype.c src/ldebug.c src/ldo.c
    src/ldump.c src/lfunc.c src/lgc.c src/llex.c src/lmem.c
    src/lobject.c src/lopcodes.c src/lparser.c src/lstate.c
    src/lstring.c src/ltable.c src/ltm.c src/lundump.c src/lvm.c src/lzio.c
)

set(LUA_LIBS
    src/lauxlib.c src/lbaselib.c src/lcorolib.c src/ldblib.c src/liolib.c
    src/lmathlib.c src/loadlib.c src/loslib.c src/lstrlib.c src/ltablib.c
    src/lutf8lib.c src/linit.c
)

set(LUA_ALL ${LUA_CORE} ${LUA_LIBS})

add_library(lua STATIC ${LUA_ALL})

target_compile_definitions(lua PRIVATE LUA_COMPAT_ALL)

if(WIN32)
    target_compile_definitions(lua PRIVATE LUA_BUILD_AS_DLL)
endif()

set_target_properties(lua PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)
