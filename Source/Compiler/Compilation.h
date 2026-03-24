#pragma once

/**
 * @file Compilation.h
 * @brief Contains the definitions of the Compilation class and related enums.
 *
 */

/**
 * @brief The CVersion enum represents the different C standards that can be used for compilation.
 */
enum class CVersion
{
    C90,
    C99,
    C11,
    C17,
    C23
};

/**
 * @brief The CppVersion enum represents the different C++ standards that can be used for compilation.
 */
enum class CppVersion
{
    CPP98,
    CPP03,
    CPP11,
    CPP14,
    CPP17,
    CPP20,
    CPP23,
    CPP26
};

/**
 * @brief The CompilationOptimisation enum represents the different optimisation levels that can be used for compilation.
 */
enum class CompilationOptimisation
{
    NONE,
    STANDARD,
    AGGRESSIVE,
    FAST,
    MIN_SIZE
};

/**
 * @brief The FloatingPointModel enum represents different floating point models for compilation.
 */
enum class FloatingPointModel
{
    STRICT,
    PRECISE,
    FAST
};

/**
 * @brief The Platform enum (flags) represents different target platforms.
 */
enum class Platform
{
    WINDOWS = 1,
    MACOS = 2,
    LINUX = 4,
    ANDROID = 8,
    IOS = 16,
    FREEBSD = 32,
    OPENBSD = 64,
    NETBSD = 128
};