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
#include "bifrost/core/hook_target.h"

namespace bifrost {

class Context;

/// Interface of the hooking implementation
class IHookMechanism {
 public:
  /// Initial set-up when the manager is created (once per process)
  virtual void SetUp(Context* ctx) = 0;

  /// Tear-down and cleanup
  virtual void TearDown(Context* ctx) = 0;

  /// Set the hook from `target` to `detour` and return the originally registered function in `original`
  virtual void SetHook(Context* ctx, const HookTarget& target, void* detour, void** original) = 0;

  /// Remove any hook that has been set to `target`
  virtual void RemoveHook(Context* ctx, const HookTarget& target) = 0;

  /// Get the type of hooking
  virtual EHookType GetType() const noexcept = 0;
};

}  // namespace bifrost