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

/// @file Bifrost shared allows fast key/value storage which can be shared between Dlls.

#include <stddef.h>  // size_t
#include <stdint.h>  // uint8_t, uint32_t, uint64_t

#pragma region Macros

/// Major version
#define BIFROST_SHARED_VERSION_MAJOR 0

/// Minor version [0-99]
#define BIFROST_SHARED_VERSION_MINOR 0

/// Patch version [0-99]
#define BIFROST_SHARED_VERSION_PATCH 1

/// Integer of the version
#define BIFROST_SHARED_VERSION (100 * (100 * BIFROST_SHARED_VERSION_MAJOR + BIFROST_SHARED_VERSION_MINOR) + BIFROST_SHARED_VERSION_PATCH)

#ifdef BIFROST_SHARED_EXPORTS
#define BIFROST_SHARED_API __declspec(dllexport)
#else
#define BIFROST_SHARED_API __declspec(dllimport)
#endif

#pragma endregion

#if __cplusplus
extern "C" {
#endif

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
/// @returns 0 if ``value`` has been assigned to the value of ``path``, 1 if ``path`` doesn't exist
BIFROST_SHARED_API int bfs_Read(const char* path, bfs_Value* value);

/// @brief Like ``bfs_Read`` but the data of ``value`` is copied
///
/// ``bfs_FreeValue`` has to be called to free potentially allocated data.
BIFROST_SHARED_API int bfs_ReadAtomic(const char* path, bfs_Value* value);

/// @brief Read the value at ``path`` and store the data in ``value``
///
/// Writes are always atomic.
///
/// @param path   Path used as a key
/// @param value  Value to use as a data
/// @return 0 if ``value`` has been written to value of ``path``, 1 otherwise
BIFROST_SHARED_API int bfs_Write(const char* path, const bfs_Value* value);

/// Get a list of all paths (should only be used for debugging)
BIFROST_SHARED_API void bfs_Paths(bfs_PathList* paths);

/// @brief Free the queried paths
BIFROST_SHARED_API void bfs_FreePaths(bfs_PathList* paths);

/// @brief Free potentially allocated data of ``value``
BIFROST_SHARED_API void bfs_FreeValue(bfs_Value* value);

/// @brief Allocates size bytes of uninitialized storage
BIFROST_SHARED_API void* bfs_Malloc(size_t value);

/// @brief Free allocated storage with ``bfs_Malloc``
BIFROST_SHARED_API void bfs_Free(void* ptr);

#if __cplusplus
}  // extern "C"
#endif
