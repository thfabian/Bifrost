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

#include "bifrost_shared/common.h"
#include "bifrost_shared/shared_object.h"
#include "bifrost_shared/shared_map.h"
#include "bifrost_shared/shared_memory.h"
#include "bifrost_shared/shared_log_stash.h"
#include "bifrost_core/logging.h"
#include <sstream>

namespace bifrost::shared {

u32 SharedObject::s_refCount = 0;
SharedObject* SharedObject::s_instance = nullptr;

SharedObject& SharedObject::Get() {
  assert(s_instance && "OnLoad() has not been called?");
  return *s_instance;
}

void SharedObject::Load(bool force) {
  if (force || s_refCount++ == 0) {
    Unload(force);
    s_instance = new SharedObject();
  }
}

void SharedObject::Unload(bool force) {
  if (force || --s_refCount == 0) {
    if (s_instance) {
      delete s_instance;
      s_instance = nullptr;
    }
  }
}

SharedObject::~SharedObject() {
  if (m_map) {
    m_map->~SharedMap();
    m_smem->Deallocate(m_map);
    m_map = nullptr;
  }

  if (m_logger) {
    m_logger->~SharedLogStash();
    m_smem->Deallocate(m_logger);
    m_logger = nullptr;
  }

  if (m_smem) {
    delete m_smem;
    m_smem = nullptr;
  }
}

SharedMemory* SharedObject::GetSharedMemory() {
  if (!m_smem) {
    // Determine size
    u32 size = BIFROST_SHARED_MEMORY_SIZE;
    const char* envSize = std::getenv("BIFROST_SHARED_MEMORY_SIZE");
    if (envSize) {
      std::stringstream ss(envSize);
      i32 sizeInt;
      ss >> sizeInt;

      if (!ss.fail() && sizeInt > 0) {
        size = sizeInt;
      }
    }

    // Determine name
    const char* name = BIFROST_SHARED_MEMORY_NAME;
    const char* envName = std::getenv("BIFROST_SHARED_MEMORY_NAME");
    if (envName) {
      name = envName;
    }

    // Allocate memory
    m_smem = new SharedMemory(SharedMemory::Config{name, size});
  }
  return m_smem;
}

SharedMap* SharedObject::GetSharedMap() {
  if (!m_map) {
    m_map = (SharedMap*)GetSharedMemory()->Allocate(sizeof(SharedMap));
    ::new (m_map) SharedMap();
  }
  return m_map;
}

SharedLogStash* SharedObject::GetSharedLogStash() {
  if (!m_logger) {
    m_logger = (SharedLogStash*)GetSharedMemory()->Allocate(sizeof(SharedLogStash));
    ::new (m_logger) SharedLogStash();
  }
  return m_logger;
}

}  // namespace bifrost::shared