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

#include <stdint.h>

#ifdef BIFROST_INJECTOR_EXPORTS
#define BIFROST_INJECTOR_API __declspec(dllexport)
#else
#define BIFROST_INJECTOR_API __declspec(dllimport)
#endif

#if __cplusplus
extern "C" {
#endif

#pragma region Version

/// @brief Major version
#define BIFROST_INJECTOR_VERSION_MAJOR 0

/// @brief Minor version [0-99]
#define BIFROST_INJECTOR_VERSION_MINOR 0

/// @brief Patch version [0-99]
#define BIFROST_INJECTOR_VERSION_PATCH 1

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

#pragma region Context

/// @brief Error status
enum bfi_Status {
  BFI_OK = 0,    ///< Everything is fine - no error
  BFI_ERROR = 1  ///< An error occurred, call bfi_GetLastError to get the message
};

/// @brief Injector context
typedef struct bfi_Context_t {
  void* _Internal;  ///< Internal pointer, do not use
} bfi_Context;

/// @brief Initialize the context
BIFROST_INJECTOR_API bfi_Context* bfi_ContextInit();

/// @brief Free the context
/// @param[in] ctx   Context description
BIFROST_INJECTOR_API void bfi_ContextFree(bfi_Context* ctx);

/// @brief Logging callback
typedef void (*bfi_LoggingCallback)(uint32_t, const char*, const char*);

/// @brief Register a logging callback `cb` - set to NULL to deregister a previously registered callback
/// @param[in] ctx  Context description
/// @param[in] cb   Logging callback, previously captured log messages will be flushed after registration
BIFROST_INJECTOR_API bfi_Status bfi_ContextSetLoggingCallback(bfi_Context* ctx, bfi_LoggingCallback cb);

/// @brief Get the last error message occurred in `ctx`
/// @param[in] ctx  Context description
BIFROST_INJECTOR_API const char* bfi_ContextGetLastError(bfi_Context* ctx);

#pragma endregion

#pragma region Process

/// @brief Executable process description, use `bfi_ProcessInject` to create a process
typedef struct bfi_Process_t {
  void* _Internal;  ///< Internal pointer, do not use
} bfi_Process;

/// @brief Free process description
/// @param[in] ctx       Context description
/// @param[in] process   Process description
BIFROST_INJECTOR_API bfi_Status bfi_ProcessFree(bfi_Context* ctx, bfi_Process* process);

/// @brief Wait for the process to complete
/// @param[in] ctx       Context description
/// @param[in] process   Process description
/// @param[out] timeout   After `timeout` seconds are elapsed, the process will be killed (set to 0 to wait forever)
/// @param[out] exitCode  The exit code of the process
BIFROST_INJECTOR_API bfi_Status bfi_ProcessWait(bfi_Context* ctx, bfi_Process* process, int32_t timeout, int32_t* exitCode);

/// @brief Poll to process to check if it is still running
/// @param[in] ctx       Context description
/// @param[in] process   Process description
/// @param[out] running   1 if the process is still running, 0 otherwise
/// @param[out] exitCode  The exit code of the process (if the process has exited)
BIFROST_INJECTOR_API bfi_Status bfi_ProcessPoll(bfi_Context* ctx, bfi_Process* process, int32_t* running, int32_t* exitCode);

/// @brief Kill the process
/// @param[in] ctx       Context description
/// @param[in] process   Process description
BIFROST_INJECTOR_API bfi_Status bfi_ProcessKill(bfi_Context* ctx, bfi_Process* process);

#pragma endregion

#pragma region Injector

/// @brief Plugin description
typedef struct bfi_Plugin_t {
  const wchar_t* Path;    ///< Absolute path to the DLL to load
  const char* Arguments;  ///< Arguments passed to the plugin
} bfi_Plugin;

/// @brief Launch or connect to the executable
enum bfi_ExecutableMode {
  BFI_UNKNOWN = 0,
  BFI_LAUNCH,            ///< Launch executable
  BFI_CONNECT_VIA_PID,   ///< Connect to process via process identifier (pid)
  BFI_CONNECT_VIA_NAME,  ///< Connect to process via name
};

/// @brief Injector Arguments
typedef struct bfi_InjectorArguments_t {
  bfi_ExecutableMode Mode;    ///< Launch or connect to the executable?
  const wchar_t* Executable;  ///< Executable to launch (requires `Mode == BFI_LAUNCH`)
  const char* Arguments;      ///< Arguments to pass to the launched executable  (requires `Mode == BFI_LAUNCH`)
  uint32_t Pid;               ///< Process identifier to connect to (requires `Mode == BFI_CONNECT_VIA_PID`)
  const wchar_t* Name;        ///< Name of the process to connect to (requires `Mode == BFI_CONNECT_VIA_NAME`)

  uint32_t InjectorTimeoutInS;  ///< Time allocated for the injecting process (in milliseconds)

  bfi_Plugin* Plugins;  ///< Plugins to inject
  uint32_t NumPlugins;  ///< Number of plugins
} bfi_InjectorArguments;

/// @brief Get default injector arguments
/// @param[in] ctx       Context description
BIFROST_INJECTOR_API bfi_InjectorArguments* bfi_InjectorArgumentsInit(bfi_Context* ctx);

/// @brief Free the injector arguments
/// @param[in] ctx   Context description
/// @param[in] args  Argument description
BIFROST_INJECTOR_API bfi_Status bfi_InjectorArgumentsFree(bfi_Context* ctx, bfi_InjectorArguments* args);

/// @brief Inject plugins into the process by connecting or launching the process
/// @param[in] ctx       Context description
/// @param[in] args      Argument description
/// @param[out] process  If injection succeeds, set to the process description
BIFROST_INJECTOR_API bfi_Status bfi_ProcessInject(bfi_Context* ctx, const bfi_InjectorArguments* args, bfi_Process_t** process);

#pragma endregion

#if __cplusplus
}  // extern "C"
#endif
