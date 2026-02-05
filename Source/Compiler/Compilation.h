#pragma once

/**
 * @file Compilation.h
 * @brief Contains the definitions of the Compilation class and related enums.
 *
 */

/**
 * @brief The CppVersion enum represents the different C++ standards that can be used for compilation.
 */
enum class CppVersion
{
    CPP_11,
    CPP_14,
    CPP_17,
    CPP_20,
    CPP_23
};

/**
 * @brief The CompilationOptimisation enum represents the different optimisation levels that can be used for compilation.
 */
enum class CompilationOptimisation
{
    NONE,
    OPTIMIZED,
    SIZE,
    SPEED
};