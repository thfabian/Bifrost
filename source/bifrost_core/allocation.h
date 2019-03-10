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

#include "bifrost_core/common.h"
#include "bifrost_core/non_copyable.h"

namespace bifrost {

/// Allocation routines
class Allocation {
 public:
  Allocation() = delete;

  class Memory : NonCopyable {
   public:
    /// Construct
    Memory(void* ptr, bool heapAllocated);
    ~Memory();

    /// Move
    Memory(Memory&& other);
    Memory& operator=(Memory&& other);

    /// Get the pointer to the start of the memory
    inline void* Ptr() { return m_ptr; }

   private:
    void* m_ptr;
    bool m_heapAllocated;
  };

  /// Allocate stack memory for
  static constexpr std::size_t MaxStackAllocationSizeInBytes = 1024;

  /// Try to allocate ``sizeInBytes`` stack memory if ``sizeInBytes`` is less than ``MaxStackAllocationSizeInBytes`` else heap allocate it
  static Memory TryStackMalloc(std::size_t sizeInBytes);

  /// Allocate ``sizeInBytes`` stack memory
  static void* StackMalloc(std::size_t sizeInBytes);
};

}  // namespace bifrost
