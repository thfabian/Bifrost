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

#include "bifrost/core/hook_vtable.h"
#include "bifrost/core/hook_debugger.h"
#include "bifrost/core/context.h"
#include "bifrost/core/ilogger.h"
#include "bifrost/core/error.h"

namespace bifrost {

namespace {

static void SetVTableHook(Context* ctx, void* target, void* detour) {
  // Unprotect
  ::MEMORY_BASIC_INFORMATION mbi;
  BIFROST_ASSERT_CALL_CTX(ctx, ::VirtualQuery((LPCVOID)target, &mbi, sizeof(mbi)) != FALSE);
  BIFROST_ASSERT_CALL_CTX(ctx, ::VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_READWRITE, &mbi.Protect) != FALSE);

  // Write the new address
  *((std::intptr_t*)target) = (std::intptr_t)detour;

  // Protect
  DWORD newProtect;
  BIFROST_ASSERT_CALL_CTX(ctx, ::VirtualProtect(mbi.BaseAddress, mbi.RegionSize, mbi.Protect, &newProtect) != FALSE);
}

}  // namespace

VTableHook::VTableHook(HookSettings* settings, HookDebugger* debugger) : HookObject(settings, debugger) {}

void VTableHook::SetUp(Context* ctx) { m_targetToOiginal.reserve(1024); }

void VTableHook::TearDown(Context* ctx) {}

void VTableHook::SetHook(Context* ctx, const HookTarget& target, void* detour, void** original) {
  BIFROST_ASSERT(target.Type == EHookType::E_VTable);

  *original = (void*)(*(std::intptr_t*)target.Target);
  m_targetToOiginal[target.Target] = (std::intptr_t)*original;

  BIFROST_HOOK_TRACE(ctx, "VTable: Creating hook from %s to %s", Sym(ctx, *original), Sym(ctx, detour));

  SetVTableHook(ctx, target.Target, detour);
}

void VTableHook::RemoveHook(Context* ctx, const HookTarget& target) {
  BIFROST_ASSERT(target.Type == EHookType::E_VTable);
  void* original = (void*)m_targetToOiginal[target.Target];

  BIFROST_HOOK_TRACE(ctx, "VTable: Removing hook from %s", Sym(ctx, original));

  SetVTableHook(ctx, target.Target, original);
  m_targetToOiginal.erase(target.Target);
}

EHookType VTableHook::GetType() const noexcept { return EHookType::E_VTable; }

}  // namespace bifrost