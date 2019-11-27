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

#define BIFROST_JUMP_TABLE_MEM_SIZE 64

namespace bifrost {

HookJumpTable::HookJumpTable(Context* ctx, HookSettings* settings, HookDebugger* debugger, IHookMechanism* mechanism, void* target)
    : HookObject(settings, debugger), m_target(target), m_tableSet(false), m_mechanism(mechanism), m_ctx(ctx) {
  BIFROST_ASSERT(mechanism->GetType() == EHookType::E_CFunction);

  // Allocate a block of executable memory
  BIFROST_ASSERT_CALL_CTX(m_ctx,
                          (m_tableEntryPoint = ::VirtualAlloc(NULL, BIFROST_JUMP_TABLE_MEM_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE)) != nullptr);
  Debugger().RegisterJumpTable(m_tableEntryPoint, m_target);
}

HookJumpTable::~HookJumpTable() {
  if (m_tableSet) RemoveJumpTarget();

  // Free the allocated block
  BIFROST_ASSERT_CALL_CTX(m_ctx, ::VirtualFree(m_tableEntryPoint, 0, MEM_RELEASE));
  Debugger().UnregisterJumpTable(m_tableEntryPoint);
}

void HookJumpTable::SetTarget(void* jumpTarget) {
  BIFROST_HOOK_TRACE(m_ctx, "Setting jump table of %s to %s", Sym(m_ctx, m_target), Sym(m_ctx, jumpTarget));

  if (m_tableSet) RemoveJumpTarget();

  // Make the entry point of the table jump to `jumpTarget`
  void* original = nullptr;

  m_mechanism->SetHook(m_ctx, {EHookType::E_CFunction, m_tableEntryPoint}, jumpTarget, &original);
  m_tableSet = true;
}

void HookJumpTable::RemoveJumpTarget() {
  BIFROST_HOOK_TRACE(m_ctx, "Removing jump table of %s", Sym(m_ctx, m_target));

  // Restore the default behavior
  m_mechanism->RemoveHook(m_ctx, {EHookType::E_CFunction, m_tableEntryPoint});
  m_tableSet = false;
}

void* HookJumpTable::GetTableEntryPoint() const {
  BIFROST_ASSERT(m_tableSet && "Jump table not set");
  return m_tableEntryPoint;
}

}  // namespace bifrost
