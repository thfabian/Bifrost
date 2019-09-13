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
  u32 GetId();

  /// Create the hook and return the pointer to the original function
  void HookCreate(u32 id, Context* ctx, void* target, void* detour, bool enable, void** original);

  /// Remove the hook
  void HookRemove(u32 id, Context* ctx, void* target);

  /// Enable the hook
  void HookEnable(u32 id, Context* ctx, void* target);

  /// Disable the hook
  void HookDisable(u32 id, Context* ctx, void* target);

	/// Enable debug mode
	void EnableDebug(Context* ctx);

 private:
  class Impl;
  std::unique_ptr<Impl> m_impl;
};

}  // namespace bifrost