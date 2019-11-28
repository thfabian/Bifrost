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
#include "bifrost/core/ihook_mechanism.h"
#include "bifrost/core/hook_object.h"

namespace bifrost {

/// VTable based hooking mechanism
class VTableHook : public IHookMechanism, public HookObject {
 public:
  VTableHook(HookSettings* settings, HookDebugger* debugger);

  virtual void SetUp(HookContext* ctx) override;
  virtual void TearDown(HookContext* ctx) override;
  virtual void SetHook(HookContext* ctx, const HookTarget& target, void* detour, void** original) override;
  virtual void RemoveHook(HookContext* ctx, const HookTarget& target) override;
  virtual EHookType GetType() const noexcept;

 private:
  std::unordered_map<void*, std::intptr_t> m_targetToOiginal;
};

}  // namespace bifrost