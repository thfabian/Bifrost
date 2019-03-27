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

namespace bifrost::shared {

/// Shared memory pool shared between server and client
class SharedMemory : public NonCopyable {
 public:
  struct Config {
    std::string Name;              ///< Name of the shared memory (used for connection)
    u64 DataSizeInBytes = 32768L;  ///< Requested size of shared memory depending on the malloc strategy the actually usable memory will be lower
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
  MallocFreeList* m_malloc;

  LPVOID m_startAddress;
  HANDLE m_handle;

  Config m_config;
};

/// STL compatible allocator
template <class T>
class SharedMemoryAllocator {
 private:
  SharedMemory* m_shared_memory;

 public:
  // Typedefs
  typedef T value_type;
  typedef value_type* pointer;
  typedef const value_type* const_pointer;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;

  inline explicit SharedMemoryAllocator(SharedMemory& shared_memory) : m_shared_memory(&shared_memory) {}
  inline SharedMemoryAllocator(const SharedMemoryAllocator<T>& other) = default;
  inline SharedMemoryAllocator(SharedMemoryAllocator<T>&& other) = default;

  // Convert an allocator<T> to allocator<U>
  template <typename U>
  struct rebind {
    typedef SharedMemoryAllocator<U> other;
  };
  template <typename U>
  inline explicit SharedMemoryAllocator(const SharedMemoryAllocator<U>& other) : m_shared_memory(other.GetSharedMemory()) {}

  // Address
  inline pointer address(reference r) { return &r; }
  inline const_pointer address(const_reference r) { return &r; }

  // Memory allocation
  [[nodiscard]] inline pointer allocate(size_type cnt) { return reinterpret_cast<pointer>(m_shared_memory->Allocate(u64(cnt) * sizeof(T))); }
  inline void deallocate(pointer p, size_type) { m_shared_memory->Deallocate(p); }

  // Size
  inline size_type max_size() const { return m_shared_memory->GetSizeInBytes() / sizeof(T); }

  // Construction/destruction
  template <class... Args>
  inline void construct(pointer p, Args&&... args) {
    ::new (p) T(std::forward<Args>(args)...);
  }
  inline void destroy(pointer p) { p->~T(); }

  inline bool operator==(SharedMemoryAllocator<T> const& other) { return m_shared_memory->GetFirstAdress() == other.GetSharedMemory()->GetFirstAdress(); }
  inline bool operator!=(SharedMemoryAllocator<T> const& other) { return !(operator==(other)); }

  SharedMemory* GetSharedMemory() const { return const_cast<SharedMemory*>(m_shared_memory); }
};

}  // namespace bifrost::shared
