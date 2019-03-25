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

/// @file API declaration of Bifrost

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

#pragma endregion

#if __cplusplus
extern "C" {
#endif

#pragma region Error

/// @brief Error status
enum bf_Status { BF_OK = 0, BF_FAIL };

/// @brief Get the explanation string of the last error (or NULL if no error occurred)
extern BIFROST_API const char* bf_GetLastError();

#pragma endregion

#pragma region Version

/// @brief Get the version string
extern BIFROST_API const char* bf_GetVersion();

#pragma endregion

#pragma region Logging

/// @brief Logging level
enum bf_LogLevel { BF_LOGLEVEL_DEBUG = 0, BF_LOGLEVEL_INFO, BF_LOGLEVEL_WARN, BF_LOGLEVEL_ERROR, BF_LOGLEVEL_DISABLE };

/// @brief Function used to sink the logs
typedef void (*bf_LogCallback_t)(int, const char*);

/// @brief Register the logging ``sink`` associated with ``name``
extern BIFROST_API bf_Status bf_RegisterLogCallback(const char* name, bf_LogCallback_t cb);

/// @brief Unregister the logging sink ``name`` if it exists
extern BIFROST_API bf_Status bf_UnregisterLogCallback(const char* name);

/// @brief Log ``message`` at ``Level``
extern BIFROST_API bf_Status bf_Log(int level, const char* message);

#pragma endregion

#pragma region Plugin

/// @brief Register the plugin ``name`` to be loaded once Bifrost gets injected
///
/// @param name         Name of the plugin
/// @param moduleName   Name of the module (DLL)
/// @param modulePath   Path to load the module from (if set to NULL the current working directory is used)
extern BIFROST_API bf_Status bf_RegisterPlugin(const char* name, const char* moduleName, const char* modulePath);

#pragma endregion

#if __cplusplus
}  // extern "C"
#endif