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
#include "bifrost/core/non_copyable.h"

namespace bifrost {

/// Shared memory interface
class ISharedMemory : public NonCopyable {
 public:
  virtual ~ISharedMemory() {}

  /// Allocates a block of size bytes of memory, returning a pointer to the beginning of the block
  virtual void* Allocate(u64 size) noexcept = 0;

  /// Deallocates the space previously allocated with `Allocate`
  virtual void Deallocate(void* ptr) noexcept = 0;

  /// Get the name of the shared memory
  virtual const char* GetName() const noexcept = 0;

  /// Get allocated size in bytes
  virtual u64 GetSizeInBytes() const noexcept = 0;

  /// Get allocated size in bytes
  virtual u64 GetNumFreeBytes() const noexcept = 0;

  /// Get the first address which can be used
  virtual const void* GetFirstAdress() const noexcept = 0;

  /// Get the base address of the shared memory
  virtual const void* GetBaseAddress() const noexcept = 0;

  /// Get offset of ``ptr`` to the base address
  virtual u64 Offset(void* ptr) const noexcept = 0;
};

}  // namespace bifrost
