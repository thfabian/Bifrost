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

#include "bifrost_core/common.h"
#include "bifrost_core/global_object.h"
#include "bifrost_core/mutex.h"
#include "bifrost_core/api_loader.h"
#include "bifrost_core/api_shared.h"
#include "bifrost_core/module_loader.h"
#include "bifrost_core/logging.h"

namespace bifrost {

std::mutex GlobalObject::s_mutex{};
GlobalObject* GlobalObject::s_instance = nullptr;

extern GlobalObject& Globals() { return GlobalObject::Get(); }

GlobalObject& GlobalObject::Get() {
  if (!s_instance) {
    BIFROST_LOCK_GUARD(s_mutex);
    if (!s_instance) {  // It could happen that 2 threads reach this point and we don't want to always lock as the condition is always false but the first time
      s_instance = new GlobalObject();
    }
  }
  return *s_instance;
}

void GlobalObject::Free() {
  BIFROST_LOCK_GUARD(s_mutex);
  if (s_instance) {
    delete s_instance;
    s_instance = nullptr;
  }
}

GlobalObject::~GlobalObject() {
#define BIFROST_GLOBAL_OBJECT_DESTRUCTOR_DEFINE(type) \
  if (m_##type) {                                     \
    delete m_##type;                                  \
    m_##type = nullptr;                               \
  }

  BIFROST_GLOBAL_OBJECT_DESTRUCTOR_DEFINE(ApiShared)
  BIFROST_GLOBAL_OBJECT_DESTRUCTOR_DEFINE(ApiLoader)
  BIFROST_GLOBAL_OBJECT_DESTRUCTOR_DEFINE(ModuleLoader)
  BIFROST_GLOBAL_OBJECT_DESTRUCTOR_DEFINE(Logging)
}

#define BIFROST_GLOBAL_OBJECT_GETTER_DEFINE(type) \
  type& GlobalObject::Get##type() {               \
    if (!m_##type) {                              \
      BIFROST_LOCK_GUARD(s_mutex);                \
      if (!m_##type) {                            \
        m_##type = new type;                      \
      }                                           \
    }                                             \
    return *m_##type;                             \
  }

BIFROST_GLOBAL_OBJECT_GETTER_DEFINE(ApiShared)
BIFROST_GLOBAL_OBJECT_GETTER_DEFINE(ApiLoader)
BIFROST_GLOBAL_OBJECT_GETTER_DEFINE(ModuleLoader)
BIFROST_GLOBAL_OBJECT_GETTER_DEFINE(Logging)

}  // namespace bifrost