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

/// @file Injector routines - all functions are thread-safe within a context.

#include <stdint.h>  // uint32_t

#ifdef BIFROST_INJECTOR_EXPORTS
#define BIFROST_INJECTOR_API __declspec(dllexport)
#else
#define BIFROST_INJECTOR_API __declspec(dllimport)
#endif

#if __cplusplus
extern "C" {
#endif

#pragma region Context

/// @brief Error status
enum bfi_Status {
  BFI_OK = 0,    ///< Everything is fine - no error
  BFI_ERROR = 1  ///< An error occurred, call bfi_GetLastError to get the message
};

/// @brief Injector context
struct bfi_Context_t {
  void* _Pointer;
};
typedef bfi_Context_t bfi_Context;

/// @brief Initialize the context
BIFROST_INJECTOR_API bfi_Context* bfi_Init();

/// @brief Free the context
BIFROST_INJECTOR_API void bfi_Free(bfi_Context* ctx);

/// @brief Get the last error message occurred in `context`
BIFROST_INJECTOR_API const char* bfi_GetLastError(bfi_Context* ctx);

/// @brief Logging callback
typedef void (*bfi_LogCallback)(uint32_t, const char*, const char*);

/// @brief Register a logging callback `cb` - set to NULL to deregister a previously registered callback
BIFROST_INJECTOR_API bfi_Status bfi_SetCallback(bfi_Context* ctx, bfi_LogCallback cb);

#pragma endregion

#pragma region Version

/// @brief Version triple
struct bfi_Version_t {
  uint32_t Major;
  uint32_t Minor;
  uint32_t Patch;
};
typedef bfi_Version_t bfi_Version;

/// @brief Get the version triple
BIFROST_INJECTOR_API bfi_Version bfi_GetVersion();

/// @brief Get the version string
BIFROST_INJECTOR_API const char* bfi_GetVersionString();

#pragma endregion

#pragma region Plugins

/// @brief Plugin description
struct bfi_Plugin_t {
  const wchar_t* Path;    ///< Absolute path to the DLL to load
  const char* Arguments;  ///< Arguments passed to the plugin
};
typedef bfi_Plugin_t bfi_Plugin;

#pragma endregion

#pragma region Injector

enum bfi_ExecutableMode {
  BFI_UNKNOWN = 0,
  BFI_LAUNCH,            ///< Launch executable
  BFI_CONNECT_VIA_PID,   ///< Connect to process via process identifier (pid)
  BFI_CONNECT_VIA_NAME,  ///< Connect to process via name
};

/// @brief Injector Arguments
struct bfi_InjectorArguments_t {
  /* Executable Process */
  bfi_ExecutableMode Mode;  ///< Launch or connect to the executable?
  const char* Executable;   ///< Executable to launch (requires `Mode == BFI_LAUNCH`)
  const char* Arguments;    ///< Arguments to pass to the launched executable  (requires `Mode == BFI_LAUNCH`)
  uint32_t Pid;             ///< Process identifier to connect to (requires `Mode == BFI_CONNECT_VIA_PID`)
  const char* Name;         ///< Name of the process to connect to (requires `Mode == BFI_CONNECT_VIA_NAME`)

  /* Injector */
  uint32_t InjectorTimeoutInMs;  ///< Time allocated for the injecting process (in milliseconds)

  /* Plugins */
  bfi_Plugin* Plugins;  ///< Plugins to inject
  uint32_t NumPlugins;  ///< Number of plugins
};
typedef bfi_InjectorArguments_t bfi_InjectorArguments;

/// @brief Get default injector arguments
BIFROST_INJECTOR_API bfi_InjectorArguments* bfi_InjectorArgumentsInit(bfi_Context* ctx);

/// @brief Inject plugins into the process by connecting or launching the process
BIFROST_INJECTOR_API bfi_Status bfi_Inject(bfi_Context* ctx, bfi_InjectorArguments* args);

/// @brief Free the arguments
BIFROST_INJECTOR_API bfi_Status bfi_InjectorArgumentsFree(bfi_Context* ctx, bfi_InjectorArguments* args);

#pragma endregion

#if __cplusplus
}  // extern "C"
#endif
