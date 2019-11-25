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

#include "bifrost/core/hook_mechanism.h"
#include "bifrost/core/hook_debugger.h"
#include "bifrost/core/context.h"
#include "bifrost/core/ilogger.h"
#include "bifrost/core/exception.h"
#include "bifrost/core/error.h"

#include "MinHook.h"

namespace bifrost {

namespace {

static const char* GetConstCharPtr(const char* str) { return str; }
static const char* GetConstCharPtr(const std::string& str) { return str.c_str(); }

}  // namespace

//
// MinHook
//

#define BIFROST_CHECK_MH(call, reason)                                                                  \
  if (MH_STATUS status; (status = call) != MH_OK) {                                                     \
    throw Exception("MinHook: Failed to %s: %s", GetConstCharPtr(reason), ::MH_StatusToString(status)); \
  }

MinHook::MinHook(HookSettings* settings, HookDebugger* debugger) : HookObject(settings, debugger) {}

void MinHook::SetUp(Context* ctx) { BIFROST_CHECK_MH(MH_Initialize(), "initialize MinHook"); }

void MinHook::TearDown(Context* ctx) { BIFROST_CHECK_MH(MH_Uninitialize(), "uninitialize MinHook"); }

void MinHook::SetHook(Context* ctx, const HookTarget& target, void* detour, void** original) {
  BIFROST_ASSERT(target.Type == EHookType::E_CFunction);
  BIFROST_HOOK_TRACE(ctx, "MinHook: Creating hook from %s to %s", Sym(ctx, target), Sym(ctx, detour));

  BIFROST_CHECK_MH(MH_CreateHook(target.CFunction.Target, detour, original), StringFormat("to create hook from %s to %s", Sym(ctx, target), Sym(ctx, detour)));
  BIFROST_CHECK_MH(MH_EnableHook(target.CFunction.Target), StringFormat("to enable hook from %s", Sym(ctx, target)));

  Debugger().RegisterTrampoline(*original, target.CFunction.Target);
}

void MinHook::RemoveHook(Context* ctx, const HookTarget& target) {
  BIFROST_ASSERT(target.Type == EHookType::E_CFunction);
  BIFROST_HOOK_TRACE(ctx, "MinHook: Removing hook from %s", Sym(ctx, target));

  BIFROST_CHECK_MH(MH_DisableHook(target.CFunction.Target), StringFormat("to disable hook from %s", Sym(ctx, target)));
  BIFROST_CHECK_MH(MH_RemoveHook(target.CFunction.Target), StringFormat("to remmove hook from %s", Sym(ctx, target)));

  Debugger().UnregisterTrampoline(target.CFunction.Target);
}

EHookType MinHook::GetType() const noexcept { return EHookType::E_CFunction; }

//
// VTable
//

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

VTableHook::VTableHook(HookSettings* settings, HookDebugger* debugger) : HookObject(settings, debugger) {}

void VTableHook::SetUp(Context* ctx) { m_targetToOiginal.reserve(1024); }

void VTableHook::TearDown(Context* ctx) {}

void VTableHook::SetHook(Context* ctx, const HookTarget& target, void* detour, void** original) {
  BIFROST_ASSERT(target.Type == EHookType::E_VTable);

  void* method = ((std::uint8_t*)target.VTable.Table) + target.VTable.Offset;
  BIFROST_HOOK_TRACE(ctx, "VTable: Creating hook from %s to %s", Sym(ctx, method), Sym(ctx, detour));

  m_targetToOiginal[method] = (std::intptr_t)method;
  SetVTableHook(ctx, method, detour);
}

void VTableHook::RemoveHook(Context* ctx, const HookTarget& target) {
  BIFROST_ASSERT(target.Type == EHookType::E_VTable);

  void* method = ((std::uint8_t*)target.VTable.Table) + target.VTable.Offset;
  BIFROST_HOOK_TRACE(ctx, "VTable: Removing hook from %s", Sym(ctx, method));

  SetVTableHook(ctx, method, (void*)m_targetToOiginal[method]);
  m_targetToOiginal.erase(method);
}

EHookType VTableHook::GetType() const noexcept { return EHookType::E_VTable; }

}  // namespace bifrost