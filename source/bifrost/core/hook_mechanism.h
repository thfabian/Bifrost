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
#include "bifrost/core/hook_object.h"

namespace bifrost {

class Context;
class HookDebugger;

/// Interface of the hooking implementation
class IHookMechanism {
 public:
  /// Initial set-up when the manager is created (once per process)
  virtual void SetUp(Context* ctx) = 0;

  /// Tear-down and cleanup
  virtual void TearDown(Context* ctx) = 0;

  /// Set the hook from `target` to `detour` and return the originally registered function in `original`
  virtual void SetHook(Context* ctx, void* target, void* detour, void** original) = 0;

  /// Remove any hook that has been set to `target`
  virtual void RemoveHook(Context* ctx, void* target) = 0;

  /// Get the type of hooking
  virtual EHookType GetType() const noexcept = 0;
};

/// C-Function hooking via Minhook
class MinHook : public IHookMechanism, public HookObject {
 public:
  MinHook(HookSettings* settings, HookDebugger* debugger);

  virtual void SetUp(Context* ctx) override;
  virtual void TearDown(Context* ctx) override;
  virtual void SetHook(Context* ctx, void* target, void* detour, void** original) override;
  virtual void RemoveHook(Context* ctx, void* target) override;
  virtual EHookType GetType() const noexcept;
};

/// VTable based hooking mechanism
class VTableHook : public IHookMechanism, public HookObject {
 public:
  VTableHook(HookSettings* settings, HookDebugger* debugger);

  virtual void SetUp(Context* ctx) override;
  virtual void TearDown(Context* ctx) override;
  virtual void SetHook(Context* ctx, void* target, void* detour, void** original) override;
  virtual void RemoveHook(Context* ctx, void* target) override;
  virtual EHookType GetType() const noexcept;
};

}  // namespace bifrost