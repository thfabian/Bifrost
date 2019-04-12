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
#include "bifrost_core/non_copyable.h"

namespace bifrost::shared {

class SharedMemory;
class SharedMap;
class SharedLogStash;

/// Per DLL instance which manages the shared memory region
class SharedObject : NonCopyable {
 public:
  /// Called when DLL is loaded - initializes per DLL SharedObject
  static void Load(bool force = false);

  /// Called when DLL is unloaded - frees per DLL SharedObject
  static void Unload(bool force = false);

  /// Get singleton instance
  static SharedObject& Get();

  /// Deallocate all memory
  ~SharedObject();

  /// Get the shared memory region
  ///
  /// The first invocation will allocate the memory - the size of shared memory can be controlled via BIFROST_SHARED_MEMORY_SIZE and the name via
  /// BIFROST_SHARED_MEMORY_NAME
  SharedMemory* GetSharedMemory();

  /// Get the shared map
  SharedMap* GetSharedMap();

  /// Get the shared logger
  SharedLogStash* GetSharedLogStash();

 private:
  static std::mutex s_mutex;
  static std::atomic<u32> s_refCount;
  static SharedObject* s_instance;

  SharedMemory* m_smem = nullptr;
  SharedMap* m_map = nullptr;
  SharedLogStash* m_logger = nullptr;
};

}  // namespace bifrost::shared