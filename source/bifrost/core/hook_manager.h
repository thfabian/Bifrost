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
#include "bifrost/core/hook_settings.h"
#include "bifrost/core/hook_target.h"

namespace bifrost {

class Context;

/// Abstraction for function and object hooking
///
/// This class should be instantiated as a per process singleton - All methods are thread-safe and throw on error.
class HookManager {
 public:
  HookManager();
  ~HookManager();

  /// Set up all internal libraries
  void SetUp(Context* ctx);

  /// Free the initialize libraries
  void TearDown(Context* ctx);

  /// Get a new unique identifier
  u32 MakeUniqueId();

  /// Set `detour` as the new function for `target` and return the originally registered function in `original`
  void SetHook(Context* ctx, u32 id, u32 priority, const HookTarget& target, void* detour, void** original);

  /// Remove the hook registered at `target`
  void RemoveHook(Context* ctx, u32 id, const HookTarget& target);

  /// Enable debug mode
  void EnableDebug(Context* ctx);

 private:
  class Impl;
  std::unique_ptr<Impl> m_impl;
};

}  // namespace bifrost