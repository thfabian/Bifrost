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

#include "bifrost_shared/common.h"
#include "bifrost_shared/malloc_freelist.h"
#include "bifrost_core/non_copyable.h"
#include <iosfwd>

namespace bifrost::shared {

/// Shared memory pool shared between server and client
class BIFROST_SHARED_API SharedMemory : public NonCopyable {
 public:
  struct Config {
    std::string Name;     ///< Name of the shared memory
    u64 DataSizeInBytes;  ///< Requested size of shared memory depending on the malloc strategy the actually usable memory will be lower
  };

  SharedMemory(const Config& config);
  ~SharedMemory();

  /// Allocates a block of size bytes of memory, returning a pointer to the beginning of the block
  void* Allocate(u64 size) noexcept { return m_malloc->Allocate(size, m_startAddress); }

  /// Deallocates the space previously allocated with `Allocate`
  void Deallocate(void* ptr) noexcept { return m_malloc->Deallocate(ptr, m_startAddress); }

  /// Get the name of the shared memory
  const char* GetName() const noexcept { return m_config.Name.c_str(); }

  /// Get allocated size in bytes
  u64 GetSizeInBytes() const noexcept { return m_config.DataSizeInBytes; }

  /// Get allocated size in bytes
  u64 GetNumFreeBytes() const noexcept { return m_malloc->GetNumFreeBytes(m_startAddress); }

  /// Get the first address which can be used
  const LPVOID GetFirstAdress() const noexcept { return m_malloc->GetFirstAdress(m_startAddress); }

  /// Get the base address of the shared memory
  const LPVOID GetBaseAddress() const noexcept { return m_startAddress; }

  /// Get the underlying malloc implementation
  MallocFreeList* GetMalloc() noexcept { return m_malloc; }

 private:
  void LogToFile(int level, std::string msg);
  void LogDebug(std::string msg);
  void LogWarn(std::string msg);
  void LogError(std::string msg);

 private:
  MallocFreeList* m_malloc;

  LPVOID m_startAddress;
  HANDLE m_handle;

  Config m_config;

  FILE* m_file;
  std::string m_module;
};

}  // namespace bifrost::shared
