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

/// @file Plugin routines.

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
typedef struct bfp_Plugin_t {
  void* _Internal;  ///< Internal pointer, do not use
} bfp_PluginContext;

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

/// @brief Set-up the plugin
/// @param[in] ctx      Plugin context description
/// @param[in] name     Name of the plugin
/// @param[in] plugin   Plugin instance
/// @param[in] param    Plugin set up parameter
BIFROST_PLUGIN_API bfp_Status bfp_PluginSetUp(bfp_PluginContext* ctx, const char* name, void* plugin, void* param);

/// @brief Tear-down the plugin
/// @param[in] ctx      Plugin context description
/// @param[in] plugin   Plugin instance
/// @param[in] param    Plugin set up parameter
BIFROST_PLUGIN_API bfp_Status bfp_PluginTearDown(bfp_PluginContext* ctx, void* plugin, void* param);

/// @brief Log the message
/// @param[in] ctx       Plugin context description
/// @param[in] severity  Plugin instance
/// @param[in] module    Plugin set up parameter
/// @param[in] msg       Plugin set up parameter
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
