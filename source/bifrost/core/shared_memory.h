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
#include "bifrost/core/malloc_freelist.h"
#include "bifrost/core/ishared_memory.h"
#include "bifrost/core/context.h"

namespace bifrost {

/// Shared memory pool shared between server and client
class SharedMemory final : public ISharedMemory {
 public:
  /// Create shared memory region ``name`` of size ``dataSizeInBytes``
  SharedMemory(Context* ctx, std::string name, u64 dataSizeInBytes);
  virtual ~SharedMemory();

  /// Allocates a block of size bytes of memory, returning a pointer to the beginning of the block
  virtual void* Allocate(u64 size) noexcept override { return m_malloc->Allocate(size, m_startAddress); }

  /// Deallocates the space previously allocated with `Allocate`
  virtual void Deallocate(void* ptr) noexcept override { return m_malloc->Deallocate(ptr, m_startAddress); }

  /// Get the name of the shared memory
  virtual const char* GetName() const noexcept override { return m_name.c_str(); }

  /// Get allocated size in bytes
  virtual u64 GetSizeInBytes() const noexcept override { return m_dataSizeInBytes; }

  /// Get allocated size in bytes
  virtual u64 GetNumFreeBytes() const noexcept override { return m_malloc->GetNumFreeBytes(m_startAddress); }

  /// Get the first address which can be used
  virtual const void* GetFirstAdress() const noexcept override { return m_malloc->GetFirstAdress(m_startAddress); }

  /// Get the base address of the shared memory
  virtual const void* GetBaseAddress() const noexcept override { return m_startAddress; }

  /// Get offset of ``ptr`` to the base address
  virtual u64 Offset(void* ptr) const noexcept override { return (u64)ptr - (u64)GetBaseAddress(); }

  /// Get the underlying malloc implementation
  MallocFreeList* GetMalloc() noexcept { return m_malloc; }

 private:
  MallocFreeList* m_malloc;

  LPVOID m_startAddress;
  HANDLE m_handle;
  std::string m_name;
  u64 m_dataSizeInBytes;

  Context* m_ctx;
};

}  // namespace bifrost
