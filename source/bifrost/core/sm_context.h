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
#include "bifrost/core/mutex.h"
#include "bifrost/core/sm_log_stash.h"
#include "bifrost/core/sm_storage.h"

namespace bifrost {

class SMContext {
 public:
  /// Create a shared context
  ///
  /// The data of the shared context will be created in the first few blocks of the shared memory
  static SMContext* Create(SharedMemory* mem, u64 memorySize);

  /// Map the shared context into an existing shared memory
  static SMContext* Map(void* firstAdress);

  /// Deallocate the context
  static void Destruct(SharedMemory* mem, SMContext* smCtx);

  /// Get the number of references to the shared memory
  u32 GetRefCount() const { return m_refCount; }

  /// Get the allocated shared memory
  u64 GetMemorySize() const { return m_memorySize; }

  /// Get the log stash
  SMLogStash* GetSMLogStash(SharedMemory* mem);

  /// Get the storage
  SMStorage* GetSMStorage(SharedMemory* mem);

 private:
  Ptr<SMStorage> m_storage;
  Ptr<SMLogStash> m_logstash;
  SpinMutex m_mutex;
  u32 m_refCount;
  u64 m_memorySize;
};

}  // namespace bifrost
