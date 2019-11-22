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

class IHookMechanism;

/// Region of memory which can be used to JMP to arbitrary functions
class HookJumpTable : HookObject {
 public:
  HookJumpTable(Context* ctx, HookSettings* settings, HookDebugger* debugger, IHookMechanism* mechanism, void* target);
  ~HookJumpTable();

  /// Set the target of the jump table, meaning if the table is called it will jump to this function
  void SetTarget(void* jumpTarget);

  /// Get the target address i.e the entry point to this jump table
  void* GetTableEntryPoint() const;

 private:
  /// Remove and existing jump target
  void RemoveJumpTarget();

 private:
  void* m_tableEntryPoint;
  void* m_target;
  bool m_tableSet;
  IHookMechanism* m_mechanism;
  Context* m_ctx;
};

}  // namespace bifrost