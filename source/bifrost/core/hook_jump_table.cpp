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

#include "bifrost/core/common.h"

#include "bifrost/core/macros.h"
#include "bifrost/core/error.h"
#include "bifrost/core/hook_jump_table.h"
#include "bifrost/core/hook_mechanism.h"
#include "bifrost/core/hook_debugger.h"
#include "bifrost/core/hook_debugger.h"

#define BIFROST_JUMP_TABLE_MEM_SIZE 64

namespace bifrost {

HookJumpTable::HookJumpTable(Context* ctx, void* target) : Object(ctx), m_target(target) {
  BIFROST_ASSERT_CALL((m_mem = ::VirtualAlloc(NULL, BIFROST_JUMP_TABLE_MEM_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE)) != nullptr);
}

HookJumpTable::~HookJumpTable() { BIFROST_ASSERT_CALL(::VirtualFree(m_mem, 0, MEM_RELEASE)); }

void HookJumpTable::SetJumpTarget(IHookMechanism* mechanism, HookDebugger* debugger, void* jumpTarget, bool verbose) {
  BIFROST_ASSERT(mechanism->GetType() == EHookType::E_CFunction);

  if (verbose) {
    Logger().DebugFormat("Setting jump table of %s to %s", debugger->SymbolFromAdress(GetContextPtr(), m_target),
      debugger->SymbolFromAdress(GetContextPtr(), jumpTarget));
  }

  if (m_tableSet) RemoveJumpTarget(mechanism, debugger);

  void* original = nullptr;
  mechanism->SetHook(GetContextPtr(), debugger, m_mem, jumpTarget, &original);
  m_tableSet = true;
}

void HookJumpTable::RemoveJumpTarget(IHookMechanism* mechanism, HookDebugger* debugger) {
  mechanism->RemoveHook(GetContextPtr(), debugger, m_mem);
  m_tableSet = false;
}

void* HookJumpTable::GetTableEntryPoint() const {
  BIFROST_ASSERT(m_tableSet && "Jump table not set");
  return m_mem;
}

}  // namespace bifrost
