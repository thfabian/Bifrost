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
#include "bifrost/core/object.h"

namespace bifrost {

class IHookMechanism;
class HookDebugger;

/// Region of memory which can be used to JMP to arbitrary functions
class HookJumpTable : Object {
 public:
  HookJumpTable(Context* ctx, void* target);
  ~HookJumpTable();

  /// Set the target of the jump table, meaning if the table is called it will jump to this function
  void SetJumpTarget(IHookMechanism* mechanism, HookDebugger* debugger, void* jumpTarget, bool verbose);

  /// Get the target address i.e the entry point to this jump table
  void* GetTableEntryPoint() const;

 private:
  void RemoveJumpTarget(IHookMechanism* mechanism, HookDebugger* debugger);

 private:
  void* m_mem = nullptr;
  void* m_target = nullptr;
  bool m_tableSet = false;
};

}  // namespace bifrost