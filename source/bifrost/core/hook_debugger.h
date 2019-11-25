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

class Context;
class HookSettings;

/// Debugging utilities for hooks
class HookDebugger : public Object {
 public:
  HookDebugger(Context* ctx, HookSettings* settings);
  ~HookDebugger();

  /// Get the name of the symbol pointed to by `addr` (requires `SymbolResolving` to be enabled, otherwise returns a string representation of the hex address)
  const char* SymbolFromAdress(Context* ctx, void* addr);
  const char* SymbolFromAdress(Context* ctx, u64 addr);

  /// Enable symbol resolving or refresh the module list if resolving is already enabled
  void EnablerOrRefreshSymbolResolving();

  /// Add a trampoline mapping for symbol resolving
  void RegisterTrampoline(void* trampoline, void* target);
  void UnregisterTrampoline(void* target);

  /// Add the jump table for symbol resolving
  void RegisterJumpTable(void* tableEntryPoint, void* target);
  void UnregisterJumpTable(void* target);

 private:
  HookSettings* m_settings;

  // Cache of address to symbol name
  std::unordered_map<u64, std::string> m_symbolCache;

  // Resolving of trampolines
  std::unordered_map<u64, u64> m_trampolineToTarget;
  std::unordered_map<u64, std::unordered_map<u64, u64>::iterator> m_targetToTrampoline;

  // Resolving jump tables
  std::unordered_map<u64, u64> m_jumpTableToTarget;
  std::unordered_map<u64, std::unordered_map<u64, u64>::iterator> m_targetToJumpTable;

  bool m_symbolResolving = false;
  bool m_init = false;
};

}  // namespace bifrost