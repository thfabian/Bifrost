//   ____  _  __               _
//  |  _ \(_)/ _|             | |
//  | |_) |_| |_ _ __ ___  ___| |_
//  |  _ <| |  _| '__/ _ \/ __| __|
//  | |_) | | | | | | (_) \__ \ |_
//  |____/|_|_| |_|  \___/|___/\__|   2018 - 2019
//
//
// This file is distributed under the MIT License (MIT).
// See LICENSE.txt for details.

#pragma once

/// @file API declaration of Bifrost---------------------

#include <stddef.h>  // size_t

#pragma region Macros

/// Major version
#define BIFROST_VERSION_MAJOR 0

/// Minor version [0-99]
#define BIFROST_VERSION_MINOR 0

/// Patch version [0-99]
#define BIFROST_VERSION_PATCH 1

/// Integer of the version
#define BIFROST_VERSION (100 * (100 * BIFROST_VERSION_MAJOR + BIFROST_VERSION_MINOR) + BIFROST_VERSION_PATCH)

#ifdef BIFROST_EXPORTS
#define BIFROST_API __declspec(dllexport)
#else
#define BIFROST_API __declspec(dllimport)
#endif

#if defined(_MSC_VER)
#define BIFROST_COMPILER_MSVC 1
#else
#define BIFROST_COMPILER_MSVC 0
#endif

#ifdef __cplusplus
#define BIFROST_CXX 1
#else
#define BIFROST_CXX 0
#endif

#if BIFROST_CXX
#define BIFROST_NOEXCEPT noexcept
#else
#define BIFROST_NOEXCEPT
#endif

#pragma endregion

#if BIFROST_CXX
extern "C" {
#endif

#pragma region Type

/// Typedef for std::size_t
typedef size_t bifrost_size_t;

#pragma endregion

#pragma region Error

/// @brief Error status
enum bifrost_Status { BIFROST_OK = 0, BIFROST_FAIL };

/// @brief Get the explanation string of the last error (or NULL if no error occurred)
extern BIFROST_API const char* bifrost_GetLastError() BIFROST_NOEXCEPT;

#pragma endregion

#pragma region Version

/// @brief Get the version string
extern BIFROST_API const char* bifrost_GetVersion() BIFROST_NOEXCEPT;

#pragma endregion

#if BIFROST_CXX
}  // extern "C"
#endif