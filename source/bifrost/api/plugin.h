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

/// @brief Type of hooks
enum bfp_HookType {
  BFP_CFUNCTION = 0,  ///< C-Function hook
  BFP_VTABLE          ///< VTable method hook
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

/// @brief Arguments required to set a hook
typedef struct bfp_HookSetDesc_t {
  bfp_HookType Type;  ///< The type of hook to use
  uint32_t Priority;  ///< The higher the priority the earlier the function will placed in the hook chain (the highest priority function will be called first)
  void* Target;       ///< A pointer to the target function, which will be overridden by the detour function
  void* Detour;       ///< A pointer to the detour function, which will override the target function
} bfp_HookSetDesc;

#define BIFROST_PLUGIN_DEFAULT_HookSetDesc_Priority (100)

/// @brief Arguments required to remove a hook
typedef struct bfp_HookRemoveDesc_t {
  bfp_HookType Type;  ///< The type of hook which was used
  void* Target;       ///< A pointer to the target function, which was used during set (used to identify the hook)
} bfp_HookRemoveDesc;

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
///
/// @param[out] errMsg   Set to NULL if no error occurred in initializing the hooking mechanism, otherwise contains the error message
BIFROST_PLUGIN_API bfp_PluginContext* bfp_PluginInit(const char** errMsg);

/// @brief Free the plugin
///
/// @param[out] errMsg   Set to NULL if no error occurred in uninitializing the hooking mechanism, otherwise contains the error message
BIFROST_PLUGIN_API void bfp_PluginFree(bfp_PluginContext* ctx, const char** errMsg);

/// @brief Free the allocated string
///
/// @param[in] str		String to free
BIFROST_PLUGIN_API void bfp_StringFree(const char* str);

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

/// @brief Get the last error message occurred in `plugin`
/// @param[in] plugin   Plugin context description
BIFROST_PLUGIN_API const char* bfp_PluginGetLastError(bfp_PluginContext* plugin);

/// @brief Creates and enables a hook for the specified target function
/// @param[in] ctx				Plugin context description
/// @param[in] desc       Description to set the hook
/// @param[out] original  A pointer to the trampoline function, which will be used to call the original target function. This parameter can be NULL.
BIFROST_PLUGIN_API bfp_Status bfp_HookSet(bfp_PluginContext* ctx, const bfp_HookSetDesc* desc, void** original);

/// @brief Removes an existing hook and restores the behavior before `bfp_HookSet` was called
/// @param[in] ctx				Plugin context description
/// @param[in] desc       Description to remove the hook
/// @param[in] target			A pointer to the target function
BIFROST_PLUGIN_API bfp_Status bfp_HookRemove(bfp_PluginContext* ctx, const bfp_HookRemoveDesc* desc);

/// @brief Enable debug mode
/// @param[in] ctx				Plugin context description
BIFROST_PLUGIN_API bfp_Status bfp_HookEnableDebug(bfp_PluginContext* ctx);

#pragma endregion

#if __cplusplus
}  // extern "C"
#endif
