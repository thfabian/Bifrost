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

#include "bifrost/core/common.h"
#include "bifrost/core/context.h"
#include "bifrost/core/malloc_freelist.h"

namespace bifrost {

class SMContext;
class SMLogStash;
class SMStorage;

/// Shared memory pool shared between processes
class SharedMemory {
 public:
  /// Create shared memory region ``name`` of size ``dataSizeInBytes``
  SharedMemory(Context* ctx, std::string name, u64 dataSizeInBytes);
  ~SharedMemory();

  /// Allocates a block of size bytes of memory, returning a pointer to the beginning of the block
  void* Allocate(u64 size) noexcept { return m_malloc->Allocate(size, m_startAddress); }

  /// Deallocates the space previously allocated with `Allocate`
  void Deallocate(void* ptr) noexcept { return m_malloc->Deallocate(ptr, m_startAddress); }

  /// Get the name of the shared memory
  const char* GetName() const noexcept { return m_name.c_str(); }

  /// Get allocated size in bytes
  u64 GetSizeInBytes() const noexcept { return m_dataSizeInBytes; }

  /// Get allocated size in bytes
  u64 GetNumFreeBytes() const noexcept { return m_malloc->GetNumFreeBytes(m_startAddress); }

  /// Get the first address which can be used
  void* GetFirstAdress() const noexcept { return m_malloc->GetFirstAdress(m_startAddress); }

  /// Get the base address of the shared memory
  void* GetBaseAddress() const noexcept { return m_startAddress; }

  /// Get offset of ``ptr`` to the base address
  u64 Offset(void* ptr) const noexcept { return (u64)ptr - (u64)GetBaseAddress(); }

  /// Get the underlying malloc implementation
  MallocFreeList* GetMalloc() noexcept { return m_malloc; }

  /// Get the shared context
  SMContext* GetSMContext() noexcept { return m_sharedCtx; }

  /// Get the log stash of SMContext
  SMLogStash* GetSMLogStash() noexcept;

  /// Get the storage of SMContext
  SMStorage* GetSMStorage() noexcept;

 private:
  MallocFreeList* m_malloc;
  SMContext* m_sharedCtx;

  LPVOID m_startAddress;
  HANDLE m_handle;
  std::string m_name;
  u64 m_dataSizeInBytes;

  Context* m_ctx;
};

}  // namespace bifrost
