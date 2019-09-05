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

/// @file Plugin routines.

#pragma once

#include <stdint.h>

#ifdef BIFROST_PLUGIN_EXPORTS
#define BIFROST_PLUGIN_API __declspec(dllexport)
#else
#define BIFROST_PLUGIN_API __declspec(dllimport)
#endif

#if __cplusplus
extern "C" {
#endif

#pragma region Enums

/// @brief Error status
enum bfp_Status {
  BFP_OK = 0,    ///< Everything is fine - no error
  BFP_ERROR = 1  ///< An error occurred, call bfp_GetLastError to get the message
};

#pragma endregion

#pragma region Structs

/// @brief Version triple
typedef struct bfp_Version_t {
  uint32_t Major;  ///< X in X.Y.Z
  uint32_t Minor;  ///< Y in X.Y.Z
  uint32_t Patch;  ///< Z in X.Y.Z
} bfp_Version;

/// @brief Injector context
typedef struct bfp_PluginContext_t {
  void* _Internal;  ///< Internal pointer, do not use
} bfp_PluginContext;

/// @brief Arguments required during SetUp
typedef struct bfp_PluginSetUpArguments_t {
  const char* Arguments;  ///< Arguments passed to the SetUp method
} bfp_PluginSetUpArguments;

/// @brief Arguments required during SetUp
typedef struct bfp_PluginTearDownArguments_t {
  bool NoFail;  ///< Allow the tear-down to fail?
} bfp_PluginTearDownArguments;

#pragma endregion

#pragma region Version

/// @brief Major version
#define BIFROST_PLUGIN_VERSION_MAJOR 0

/// @brief Minor version [0-99]
#define BIFROST_PLUGIN_VERSION_MINOR 0

/// @brief Patch version [0-99]
#define BIFROST_PLUGIN_VERSION_PATCH 1

/// @brief Get the version triple
BIFROST_PLUGIN_API bfp_Version_t bfp_GetVersion(void);

/// @brief Get the version string
BIFROST_PLUGIN_API const char* bfp_GetVersionString(void);

#pragma endregion

#pragma region Plugin

/// @brief Initialize the plugin
BIFROST_PLUGIN_API bfp_PluginContext* bfp_PluginInit(void);

/// @brief Start the set-up process of the plugin
/// @param[in] ctx      Plugin context description
/// @param[in] name     Name of the plugin
/// @param[in] param    Plugin set up parameter, this has to of type PluginContext::SetUpParam and is constructed by the loader
/// @param[out] args    Extracted arguments from `param`
BIFROST_PLUGIN_API bfp_Status bfp_PluginSetUpStart(bfp_PluginContext* ctx, const char* name, const void* param, bfp_PluginSetUpArguments** args);

/// @brief Finalize the set-up process of the plugin
/// @param[in] ctx      Plugin context description
/// @param[in] name     Name of the plugin
/// @param[in] param    Plugin set up parameter, this has to of type PluginContext::SetUpParam and is constructed by the loader
/// @param[in] args     Arguments allocated by bfp_PluginSetUpStart (will free the memory)
BIFROST_PLUGIN_API bfp_Status bfp_PluginSetUpEnd(bfp_PluginContext* ctx, const char* name, const void* param, const bfp_PluginSetUpArguments* args);

/// @brief Start the tear-down process of the plugin
/// @param[in] ctx      Plugin context description
/// @param[in] param    Plugin set up parameter, this has to of type PluginContext::TearDownParam and is constructed by the loader
/// @param[out] args    Extracted arguments from `param`
BIFROST_PLUGIN_API bfp_Status bfp_PluginTearDownStart(bfp_PluginContext* ctx, const void* param, bfp_PluginTearDownArguments** args);

/// @brief Finalize the tear-down process of the plugin
/// @param[in] ctx      Plugin context description
/// @param[in] param    Plugin set up parameter, this has to of type PluginContext::TearDownParam and is constructed by the loader
/// @param[in] args     Arguments allocated by bfp_PluginTearDownStart (will free the memory)
BIFROST_PLUGIN_API bfp_Status bfp_PluginTearDownEnd(bfp_PluginContext* ctx, const void* param, const bfp_PluginTearDownArguments* args);

/// @brief Log the message
/// @param[in] ctx       Plugin context description
/// @param[in] severity  Severity of the log message
/// @param[in] module    Module to log from
/// @param[in] msg       Log message
BIFROST_PLUGIN_API bfp_Status bfp_PluginLog(bfp_PluginContext* ctx, uint32_t level, const char* module, const char* msg);

/// @brief Free the plugin
/// @param[in] plugin   Plugin context description
BIFROST_PLUGIN_API void bfp_PluginFree(bfp_PluginContext* ctx);

/// @brief Get the last error message occurred in `plugin`
/// @param[in] plugin   Plugin context description
BIFROST_PLUGIN_API const char* bfp_PluginGetLastError(bfp_PluginContext* plugin);

#pragma endregion

#if __cplusplus
}  // extern "C"
#endif
