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

#include "bifrost/core/context.h"
#include "bifrost/core/ilogger.h"
#include "bifrost/core/shared_memory.h"
#include "bifrost/core/ptr.h"

namespace bifrost {

/// Base class of all Bifrost objects
class Object {
 public:
  Object(Context& ctx) : m_ctx(&ctx) {}
  Object(Context* ctx) : m_ctx(ctx) {}
  Object(Object* obj) : m_ctx(&obj->GetContext()) {}

  /// Access the context
  inline Context& GetContext() { return *m_ctx; }
  inline const Context& GetContext() const { return *m_ctx; }
  inline Context* GetContextPtr() { return m_ctx; }
  inline const Context* GetContextPtr() const { return m_ctx; }

  /// Get the logger
  inline ILogger& Logger() { return m_ctx->Logger(); }
  inline const ILogger& Logger() const { return m_ctx->Logger(); }

  /// Get shared memory
  inline SharedMemory& Memory() { return m_ctx->Memory(); }
  inline const SharedMemory& Memory() const { return m_ctx->Memory(); }

  /// Resolve the given pointer
  template <class T>
  inline T* Resolve(Ptr<T> ptr) {
    return ptr.Resolve((void*)Memory().GetBaseAddress());
  }
  template <class T>
  inline T* Resolve(const Ptr<T>& ptr) const {
    return ptr.Resolve((void*)Memory().GetBaseAddress());
  }

 private:
  Context* m_ctx;
};

}  // namespace bifrost