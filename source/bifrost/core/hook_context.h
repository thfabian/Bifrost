
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
#include "bifrost/core/type.h"
#include "bifrost/core/thread.h"

namespace bifrost {

class Context;

/// Context of setting a hook
struct HookContext {
  Context* Context;                                     ///< Bifrost Context
  std::vector<std::unique_ptr<Thread>>* FrozenThreads;  ///< Reference to the suspended threads
};

}  // namespace bifrost
