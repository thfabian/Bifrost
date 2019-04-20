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

/// @file Bifrost shared allows fast key/value storage which can be shared system wide.

#include <stddef.h>  // size_t
#include <stdint.h>  // uint8_t, uint32_t, uint64_t

#pragma region Macros

#ifdef BIFROST_SHARED_EXPORTS
#define BIFROST_SHARED_API __declspec(dllexport)
#else
#define BIFROST_SHARED_API __declspec(dllimport)
#endif

#pragma endregion

#if __cplusplus
extern "C" {
#endif

#pragma region Version

struct bfs_Version {
  uint32_t Major;
  uint32_t Minor;
  uint32_t Patch;
};

/// @brief Get the version
/// @remarks
///    This function is thread-safe.
BIFROST_SHARED_API bfs_Version bfs_GetVersion();

/// @brief Get the version string
/// @remarks
///    This function is thread-safe.
BIFROST_SHARED_API const char* bfs_GetVersionString();

#pragma endregion

#pragma region Context


#pragma endregion


/*
#pragma region Error Handling

/// @brief ErrorStash status
enum bfs_Status {
  BFS_OK = 0,          ///< Everything is fine - no error
  BFS_PATH_NOT_EXIST,  ///< Path does not exists
  BFS_OUT_OF_MEMORY,   ///< Out of shared memory
  BFS_UNKNOWN,         ///< Unknown error
};

/// @brief Convert the status to string
/// @remarks
///    This function is thread-safe.
BIFROST_SHARED_API const char* bfs_StatusString(bfs_Status status);

#pragma endregion

#pragma region Shared Storage

/// @brief Supported types of values
enum bfs_Type {
  BFS_BOOL = 0,  ///< Boolean (1 byte)
  BFS_INT,       ///< Integer (4 bytes)
  BFS_DOUBLE,    ///< Double precision floating point (8 bytes)
  BFS_STRING,    ///< String
  BFS_BYTE,      ///< Raw bytes
};

/// @brief Value description
#pragma pack(push, 1)
struct bfs_Value_t {
  uint8_t Type;                     ///< Enum of type ``bfs_Type``
  uint64_t Value;                   ///< Value or pointer to the string
  uint32_t SizeInBytes;             ///< Size of bytes
  uint8_t Padding[64 - 8 - 1 - 4];  ///< Padding (can be used to store string data)
};
#pragma pack(pop)
typedef bfs_Value_t bfs_Value;

/// @brief List of keys
struct bfs_PathList_t {
  uint32_t Num;    ///< Number of paths
  uint32_t* Lens;  ///< Length of each path (``Lens`` is of size ``Num``)
  char** Paths;    ///< List of all paths (``Lens`` is of size ``Num``)
};
typedef bfs_PathList_t bfs_PathList;

/// @brief Read the value at ``path`` and store the data in ``value``
///
/// For sufficiently large strings a reference will be stored - use ``bfs_ReadAtomic`` to get a copy of the data in any case.
///
/// @param path   Path used as a key
/// @param value  Value of data of ``path``
/// @remarks
///    This function is thread-safe.
BIFROST_SHARED_API bfs_Status bfs_Read(const char* path, bfs_Value* value);

/// @brief Like ``bfs_Read`` but the data of ``value`` is copied
///
/// ``bfs_FreeValue`` has to be called to free potentially allocated data.
/// @remarks
///    This function is thread-safe.
BIFROST_SHARED_API bfs_Status bfs_ReadAtomic(const char* path, bfs_Value* value);

/// @brief Read the value at ``path`` and store the data in ``value``
///
/// Writes are always atomic.
///
/// @param path   Path used as a key
/// @param value  Value to use as a data
/// @remarks
///    This function is thread-safe.
BIFROST_SHARED_API bfs_Status bfs_Write(const char* path, const bfs_Value* value);

/// @brief Get a list of all paths (should only be used for debugging)
/// @remarks
///    This function is thread-safe.
BIFROST_SHARED_API bfs_Status bfs_Paths(bfs_PathList* paths);

/// @brief Deallocate the queried paths
/// @remarks
///    This function is thread-safe.
BIFROST_SHARED_API void bfs_FreePaths(bfs_PathList* paths);

/// @brief Deallocate potentially allocated data of ``value``
BIFROST_SHARED_API void bfs_FreeValue(bfs_Value* value);

/// @brief Allocates size bytes of uninitialized storage
/// @remarks
///    This function is thread-safe.
BIFROST_SHARED_API void* bfs_Malloc(size_t value);

/// @brief Deallocate allocated storage with ``bfs_Malloc``
/// @remarks
///    This function is thread-safe.
BIFROST_SHARED_API void bfs_Free(void* ptr);

/// @brief Get the unused number of bytes in the shared memory region
/// @remarks
///    This function is thread-safe.
BIFROST_SHARED_API uint32_t bfs_NumBytesUnused();

/// @brief Get the total number of bytes mapped in the shared memory region
/// @remarks
///    This function is thread-safe.
BIFROST_SHARED_API uint32_t bfs_NumBytesTotal();

/// @brief Reset all allocated shared memory
/// @remarks
///    This function is thread-safe.
BIFROST_SHARED_API bfs_Status bfs_Reset();

#pragma endregion

#pragma region Logging

/// @brief Logging level
enum bfs_LogLevel { BFS_LOGLEVEL_DEBUG = 0, BFS_LOGLEVEL_INFO, BFS_LOGLEVEL_WARN, BFS_LOGLEVEL_ERROR, BFS_LOGLEVEL_DISABLE };

/// @brief Function used to sink the logs
typedef void (*bfs_LogCallback_t)(int, const char*, const char*);

/// @brief Register the logging ``sink`` associated with ``name``
/// @remarks
///    This function is thread-safe.
BIFROST_SHARED_API bfs_Status bfs_RegisterLogCallback(const char* name, bfs_LogCallback_t cb);

/// @brief Unregister the logging sink ``name`` if it exists
/// @remarks
///    This function is thread-safe.
BIFROST_SHARED_API bfs_Status bfs_UnregisterLogCallback(const char* name);

/// @brief Log ``message`` at ``level``
/// @remarks
///    This function is thread-safe.
BIFROST_SHARED_API bfs_Status bfs_Log(int level, const char* module, const char* message);

/// @brief Log ``message`` at ``level``
/// @remarks
///    This function is thread-safe.
BIFROST_SHARED_API bfs_Status bfs_LogStateAsync(int async);

#pragma endregion*/

#if __cplusplus
}  // extern "C"
#endif