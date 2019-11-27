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

/// C-Function hooking
class CFunctionHook : public IHookMechanism, public HookObject {
 public:
  CFunctionHook(HookSettings* settings, HookDebugger* debugger);

  virtual void SetUp(Context* ctx) override;
  virtual void TearDown(Context* ctx) override;
  virtual void SetHook(Context* ctx, const HookTarget& target, void* detour, void** original) override;
  virtual void RemoveHook(Context* ctx, const HookTarget& target) override;
  virtual EHookType GetType() const noexcept;
};

}  // namespace bifrost