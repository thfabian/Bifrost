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

namespace bifrost {

/// Base class of all Bifrost objects
class Object {
 public:
  Object(Context& ctx) : m_ctx(&ctx) {}
  Object(Context* ctx) : m_ctx(ctx) {}
  Object(Object* obj) : m_ctx(&obj->GetContext()) {}

  virtual ~Object() {}

  /// Access the context
  inline Context& GetContext() { return *m_ctx; }
  inline const Context& GetContext() const { return *m_ctx; }

  /// Get the logger
  inline ILogger& Logger() { return m_ctx->Logger(); }
  inline const ILogger& Logger() const { return m_ctx->Logger(); }

 private:
  Context* m_ctx;
};

}  // namespace bifrost