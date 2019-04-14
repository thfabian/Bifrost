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

/// @file Bifrost Loader injects itself in the target application and loads the plugins

#pragma region Macros

/// Major version
#define BIFROST_LOADER_VERSION_MAJOR 0

/// Minor version [0-99]
#define BIFROST_LOADER_VERSION_MINOR 0

/// Patch version [0-99]
#define BIFROST_LOADER_VERSION_PATCH 1

/// Integer of the version
#define BIFROST_LOADER_VERSION (100 * (100 * BIFROST_LOADER_VERSION_MAJOR + BIFROST_LOADER_VERSION_MINOR) + BIFROST_LOADER_VERSION_PATCH)

#ifdef BIFROST_LOADER_EXPORTS
#define BIFROST_LOADER_API __declspec(dllexport)
#else
#define BIFROST_LOADER_API __declspec(dllimport)
#endif

/// @brief Prefix for shared storage keys
#define BFL(Key) "bfl." Key

#pragma endregion

#if __cplusplus
extern "C" {
#endif

#pragma region Error Handling

/// @brief ErrorStash status
enum bfl_Status {
  BFL_OK = 0,         ///< Everything is fine - no error
  BFL_OUT_OF_MEMORY,  ///< Out of shared memory
  BFL_UNKNOWN,        ///< Unknown error
};

/// @brief Convert the status to string
BIFROST_LOADER_API const char* bfl_StatusString(bfl_Status status);

#pragma endregion

#pragma region Version

/// @brief Get the version string
BIFROST_LOADER_API const char* bfl_GetVersion();

#pragma endregion

#pragma region Plugin

/// @brief Plugin description
struct bfl_Plugin_t {
  const char* Name;           ///< Name of the plugin
  const char* DllName;        ///< Name of the DLL
  const char* DllSearchPath;  ///< Path to load the DLL from (if NULL the default search path will be used)
};
typedef bfl_Plugin_t bfl_Plugin;

/// @brief Load all plugins into the executable with the given PID
/// @remarks
///    This function is thread-safe.
BIFROST_LOADER_API bfl_Status bfl_Reset();

/// @brief Register a new plugin which will be loaded when bifrost_loader is injected in the registered executable
///
/// @param plugin  Plugin description
/// @remarks
///    This function is thread-safe.
BIFROST_LOADER_API bfl_Status bfl_RegisterPlugin(const bfl_Plugin* plugin);

/// @brief Load all plugins into the executable with the given PID
///
/// @param pid  Process identifier
/// @remarks
///    This function is thread-safe.
BIFROST_LOADER_API bfl_Status bfl_RegisterExecutable(int pid);

#pragma endregion

#if __cplusplus
}  // extern "C"
#endif
