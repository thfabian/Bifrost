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

  /// Description of setting a hook
  struct SetDesc {
    u32 Priority;       ///< The higher the priority the earlier the function will placed in the hook chain (the highest priority function will be called first)
    HookTarget Target;  ///< A pointer to the target function which will be overridden by the detour function - also includes the type of hook
    void* Detour;       ///< A pointer to the detour function, which will override the target function
  };

  /// Result of setting a hook
  struct SetResult {
    void* Original;  ///< A pointer to the trampoline function which will can be used to call the original target function
  };

  /// Set `SetDesc::Detour` as the new function for `SetDesc::Target` and return the originally registered function in `SetResult::Original`
  std::vector<SetResult> SetHooks(Context* ctx, u32 id, const SetDesc* descs, u32 num);

  /// Description of setting a hook
  struct RemoveDesc {
    HookTarget Target;  ///< A pointer to the target function for which the hook was applied - also includes the type of hook
  };

  /// Remove the hook registered at `RemoveDesc::target`
  void RemoveHooks(Context* ctx, u32 id, const RemoveDesc* descs, u32 num);

  /// Enable debug mode
  void EnableDebug(Context* ctx);

 private:
  class Impl;
  std::unique_ptr<Impl> m_impl;
};

}  // namespace bifrost