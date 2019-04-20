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
#include "bifrost/core/ptr.h"
#include "bifrost/core/context.h"
#include "bifrost/core/shared_memory.h"
#include "bifrost/core/macros.h"

namespace bifrost {

class SMObject {
 public:
  /// Resolve the given pointer
  template <class T>
  inline T* Resolve(Context* ctx, Ptr<T> ptr) {
    return ptr.Resolve((void*)ctx->Memory().GetBaseAddress());
  }
  template <class T>
  inline T* Resolve(Context* ctx, const Ptr<T>& ptr) const {
    return ptr.Resolve((void*)ctx->Memory().GetBaseAddress());
  }

  /// Destruct the object (use instead of destructor - called by `Delete` and `DeleteArray`)
  void Destruct(SharedMemory* mem) { BIFROST_ASSERT(false && "Destructor not implemented"); }
};

}  // namespace bifrost