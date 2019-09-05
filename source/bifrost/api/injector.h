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

/// @file Injector routines.

#pragma once

#include <stdint.h>

#ifdef BIFROST_INJECTOR_EXPORTS
#define BIFROST_INJECTOR_API __declspec(dllexport)
#else
#define BIFROST_INJECTOR_API __declspec(dllimport)
#endif

#if __cplusplus
extern "C" {
#endif

#pragma region Enums

/// @brief Error status
enum bfi_Status {
  BFP_OK = 0,    ///< Everything is fine - no error
  BFP_ERROR = 1  ///< An error occurred, call bfi_GetLastError to get the message
};

/// @brief Launch or connect to the executable
enum bfi_ExecutableMode {
  BFI_UNKNOWN = 0,
  BFI_LAUNCH,            ///< Launch executable
  BFI_CONNECT_VIA_PID,   ///< Connect to process via process identifier (pid)
  BFI_CONNECT_VIA_NAME,  ///< Connect to process via name
};

#pragma endregion

#pragma region Structs

/// @brief Version triple
typedef struct bfi_Version_t {
  uint32_t Major;  ///< X in X.Y.Z
  uint32_t Minor;  ///< Y in X.Y.Z
  uint32_t Patch;  ///< Z in X.Y.Z
} bfi_Version;

/// @brief Executable process description, use `bfi_ProcessInject` to create a process
typedef struct bfi_Process_t {
  void* _Internal;  ///< Internal pointer, do not use
} bfi_Process;

/// @brief Injector context
typedef struct bfi_Context_t {
  void* _Internal;  ///< Internal pointer, do not use
} bfi_Context;

/// @brief Executable description
typedef struct bfi_Executable_t {
  bfi_ExecutableMode Mode;          ///< Launch or connect to the executable?
  const wchar_t* ExecutablePath;    ///< Executable to launch (requires `Mode == BFI_LAUNCH`)
  const char* ExecutableArguments;  ///< Arguments to pass to the launched executable  (requires `Mode == BFI_LAUNCH`)
  uint32_t Pid;                     ///< Process identifier to connect to (requires `Mode == BFI_CONNECT_VIA_PID`)
  const wchar_t* Name;              ///< Name of the process to connect to (requires `Mode == BFI_CONNECT_VIA_NAME`)
} bfi_ExecutableArguments;

#define BIFROST_INJECTOR_DEFAULT_InjectorArguments_SharedMemorySizeInBytes (1 << 22)
#define BIFROST_INJECTOR_DEFAULT_InjectorArguments_TimeoutInS (5)

/// @brief Injector description
typedef struct bfi_InjectorArguments_t {
  uint32_t TimeoutInS;               ///< Time allocated for the injecting process (in milliseconds)
  const char* SharedMemoryName;      ///< Name of shared memory
  uint32_t SharedMemorySizeInBytes;  ///< Size of shared memory
  uint32_t Debugger;                 ///< Attach a Visual Studio debugger?
  const wchar_t* VSSolution;         ///< Connect to the Visual Studio instance which has `VSolution` open - if set to NULL any of them is used
} bfi_InjectorArguments;

#define BIFROST_INJECTOR_DEFAULT_PluginLoadDesc_ForceLoad 0

/// @brief Plugin description
typedef struct bfi_PluginLoadDesc_t {
  const char* Name;       ///< Name of the plugin
  const wchar_t* Path;    ///< Absolute path to the DLL to load
  const char* Arguments;  ///< Arguments passed to the plugin
  uint32_t ForceLoad;     ///< Set to 1 to force the plugin to be loaded even if it has already been loaded. Meaning, the plugin will be unloaded first.
} bfi_PluginLoadDesc;

/// @brief Plugin load arguments
typedef struct bfi_PluginLoadArguments_t {
  bfi_ExecutableArguments* Executable;       ///< Executable to launch or connect
  bfi_InjectorArguments* InjectorArguments;  ///< Arguments to the injector process
  bfi_PluginLoadDesc* Plugins;               ///< Plugins to load
  uint32_t NumPlugins;                       ///< Number of plugins
} bfi_PluginLoadArguments;

/// @brief Result of loading plugins
typedef struct bfi_PluginLoadResult_t {
  const char* SharedMemoryName;  ///< Name of the used shared memory
  uint32_t SharedMemorySize;     ///< Size of the used shared memory
  uint32_t RemoteProcessPid;     ///< Process identifier of the launched or connected process
} bfi_PluginLoadResult;

/// @brief Plugin description
typedef struct bfi_PluginUnloadDesc_t {
  const char* Name;  ///< Name of the plugin
} bfi_PluginUnloadDesc;

#define BIFROST_INJECTOR_DEFAULT_PluginUnloadArguments_UnloadAll 0

/// @brief Plugin load arguments
typedef struct bfi_PluginUnloadArguments_t {
  bfi_InjectorArguments* InjectorArguments;  ///< Arguments to the injector process
  bfi_PluginUnloadDesc* Plugins;             ///< Plugins to unload
  uint32_t NumPlugins;                       ///< Number of plugins
  uint32_t UnloadAll;                        ///< Set to 1 to unload all plugins (ignores `Plugins` argument)
} bfi_PluginUnloadArguments;

/// @brief Result of unloading plugins
typedef struct bfi_PluginUnloadResult_t {
  int32_t* Unloaded;  ///< Set to 1 if the plugin has been successfully unloaded, 0 otherwise - (size `bfi_PluginUnloadArguments.NumPlugins`)
} bfi_PluginUnloadResult;

/// @brief Plugin message arguments
typedef struct bfi_PluginMessageArguments_t {
  bfi_InjectorArguments* InjectorArguments;  ///< Arguments to the injector process
  const char* PluginName;                    ///< Name of the receiving plugin
  const void* MessageData;                   ///< Data of the message
  const void* MessageSizeInBytes;            ///< Size of the message in bytes
} bfi_PluginMessageArguments;

#pragma endregion

#pragma region Version

/// @brief Major version
#define BIFROST_INJECTOR_VERSION_MAJOR 0

/// @brief Minor version [0-99]
#define BIFROST_INJECTOR_VERSION_MINOR 0

/// @brief Patch version [0-99]
#define BIFROST_INJECTOR_VERSION_PATCH 1

/// @brief Get the version triple
BIFROST_INJECTOR_API bfi_Version bfi_GetVersion(void);

/// @brief Get the version string
BIFROST_INJECTOR_API const char* bfi_GetVersionString(void);

#pragma endregion

#pragma region Context

/// @brief Initialize the context
BIFROST_INJECTOR_API bfi_Context* bfi_ContextInit(void);

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

/// @brief Launch the process given by executable `path`
/// @param[in] ctx       Context description
/// @param[in] path      Path to the executable
/// @param[in] args      Arguments passed to the executable
/// @param[out] process  Process description
BIFROST_INJECTOR_API bfi_Status bfi_ProcessLaunch(bfi_Context* ctx, const wchar_t* path, const char* arguments, bfi_Process_t** process);

/// @brief Connect to the process given by `pid`
/// @param[in] ctx       Context description
/// @param[in] pid       Process identifier
/// @param[out] process  Process description
BIFROST_INJECTOR_API bfi_Status bfi_ProcessFromPid(bfi_Context* ctx, uint32_t pid, bfi_Process_t** process);

/// @brief Connect to the process given by `name`
/// @param[in] ctx       Context description
/// @param[in] name      Name of the process
/// @param[out] process  Process description
BIFROST_INJECTOR_API bfi_Status bfi_ProcessFromName(bfi_Context* ctx, const wchar_t* name, bfi_Process_t** process);

/// @brief Free process description
/// @param[in] ctx       Context description
/// @param[in] process   Process description
BIFROST_INJECTOR_API bfi_Status bfi_ProcessFree(bfi_Context* ctx, bfi_Process* process);

/// @brief Wait for the process to complete
/// @param[in] ctx        Context description
/// @param[in] process    Process description
/// @param[out] timeout   After `timeout` seconds are elapsed, the process will be killed (set to 0 to wait forever)
/// @param[out] exitCode  The exit code of the process
BIFROST_INJECTOR_API bfi_Status bfi_ProcessWait(bfi_Context* ctx, bfi_Process* process, int32_t timeout, int32_t* exitCode);

/// @brief Poll to process to check if it is still running
/// @param[in] ctx        Context description
/// @param[in] process    Process description
/// @param[out] running   1 if the process is still running, 0 otherwise
/// @param[out] exitCode  The exit code of the process (if the process has exited)
BIFROST_INJECTOR_API bfi_Status bfi_ProcessPoll(bfi_Context* ctx, bfi_Process* process, int32_t* running, int32_t* exitCode);

/// @brief Kill the process
/// @param[in] ctx       Context description
/// @param[in] process   Process description
BIFROST_INJECTOR_API bfi_Status bfi_ProcessKill(bfi_Context* ctx, bfi_Process* process);

/// @brief Kill all process identified by `name`
/// @param[in] ctx    Context description
/// @param[in] name   Name of the process
BIFROST_INJECTOR_API bfi_Status bfi_ProcessKillByName(bfi_Context* ctx, const wchar_t* name);

/// @brief Kill the process identified by `pid`
/// @param[in] ctx    Context description
/// @param[in] pid    Process identifier
BIFROST_INJECTOR_API bfi_Status bfi_ProcessKillByPid(bfi_Context* ctx, uint32_t pid);

#pragma endregion

#pragma region Plugin

/// @brief Load the plugins into the process by connecting or launching the executable
/// @param[in] ctx       Context description
/// @param[in] args      Argument description
/// @param[out] process  If injection succeeds, set to the process description
/// @param[out] result   Result of the plugin loading
BIFROST_INJECTOR_API bfi_Status bfi_PluginLoad(bfi_Context* ctx, const bfi_PluginLoadArguments* args, bfi_Process_t** process, bfi_PluginLoadResult** result);

/// @brief Free plugin load result
/// @param[in] ctx      Context description
/// @param[in] result   Result of the plugin loading
BIFROST_INJECTOR_API bfi_Status bfi_PluginLoadResultFree(bfi_Context* ctx, bfi_PluginLoadResult* result);

/// @brief Unload the plugins from the process
/// @param[in] ctx       Context description
/// @param[in] args      Argument description
/// @param[in] process   Process to unload the plugins from
/// @param[out] result   Result of the plugin unloading
BIFROST_INJECTOR_API bfi_Status bfi_PluginUnload(bfi_Context* ctx, const bfi_PluginUnloadArguments* args, const bfi_Process_t* process,
                                                 bfi_PluginUnloadResult** result);

/// @brief Free plugin unload result
/// @param[in] ctx      Context description
/// @param[in] result   Result of the plugin unloading
BIFROST_INJECTOR_API bfi_Status bfi_PluginUnloadResultFree(bfi_Context* ctx, bfi_PluginUnloadResult* result);

/// @brief Send a message to the plugin
/// @param[in] ctx    Context description
/// @param[in] name   Name of the plugin
/// @param[in] data   Start of message
/// @param[in] size   Size of message in bytes
BIFROST_INJECTOR_API bfi_Status bfi_PluginMessage(bfi_Context* ctx, const bfi_PluginMessageArguments* args, const bfi_Process_t* process);

/// @brief Get the help description of the plugin
/// @param[in] ctx     Context description
/// @param[in] path    Path to the dll of the plugin
/// @param[out] help   '\0' terminated help message string, use `bfi_PluginHelpFree` to deallocate it
BIFROST_INJECTOR_API bfi_Status bfi_PluginHelp(bfi_Context* ctx, const wchar_t* path, char** help);

/// @brief Get the help description of the plugin
/// @param[in] ctx     Context description
/// @param[out] help   Help string to deallocate
BIFROST_INJECTOR_API bfi_Status bfi_PluginHelpFree(bfi_Context* ctx, char* help);

#pragma endregion

#if __cplusplus
}  // extern "C"
#endif
