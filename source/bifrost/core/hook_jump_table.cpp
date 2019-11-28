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
#include "bifrost/core/ihook_mechanism.h"
#include "bifrost/core/hook_jump_table.h"

#define BIFROST_JUMP_TABLE_MEM_SIZE 64

namespace bifrost {

HookJumpTable::HookJumpTable(HookContext* ctx, HookSettings* settings, HookDebugger* debugger, IHookMechanism* mechanism, void* target)
    : HookObject(settings, debugger), m_target(target), m_tableSet(false), m_mechanism(mechanism) {
  BIFROST_ASSERT(mechanism->GetType() == EHookType::E_CFunction);

  // Allocate a block of executable memory
  BIFROST_ASSERT_CALL_CTX(ctx->Context,
                          (m_tableEntryPoint = ::VirtualAlloc(NULL, BIFROST_JUMP_TABLE_MEM_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE)) != nullptr);
  Debugger().RegisterJumpTable(m_tableEntryPoint, m_target);
}

HookJumpTable::~HookJumpTable() {
  BIFROST_ASSERT(!m_tableSet && "Jump table was not freed");

  // Free the allocated block
  ::VirtualFree(m_tableEntryPoint, 0, MEM_RELEASE);
  Debugger().UnregisterJumpTable(m_tableEntryPoint);
}

void HookJumpTable::SetTarget(HookContext* ctx, void* jumpTarget) {
  BIFROST_HOOK_TRACE(ctx->Context, "Setting jump table of %s to %s", Sym(ctx, m_target), Sym(ctx, jumpTarget));

  if (m_tableSet) RemoveTarget(ctx);

  // Make the entry point of the table jump to `jumpTarget`
  void* original = nullptr;
  m_mechanism->SetHook(ctx, {EHookType::E_CFunction, m_tableEntryPoint}, jumpTarget, &original);
  m_tableSet = true;
}

void HookJumpTable::RemoveTarget(HookContext* ctx) {
  BIFROST_HOOK_TRACE(ctx->Context, "Removing jump table of %s", Sym(ctx, m_target));

  // Restore the default behavior
  m_mechanism->RemoveHook(ctx, {EHookType::E_CFunction, m_tableEntryPoint});
  m_tableSet = false;
}

void* HookJumpTable::GetTableEntryPoint() const {
  BIFROST_ASSERT(m_tableSet && "Jump table not set");
  return m_tableEntryPoint;
}

}  // namespace bifrost
