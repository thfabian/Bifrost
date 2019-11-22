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

#define BIFROST_CHECK_MH(call, reason)                                                         \
  if (MH_STATUS status; (status = call) != MH_OK) {                                            \
    throw Exception("MinHook: Failed to %s: %s", GetConstCharPtr(reason), ::MH_StatusToString(status)); \
  }

namespace bifrost {

namespace {

static const char* GetConstCharPtr(const char* str) { return str; }
static const char* GetConstCharPtr(const std::string& str) { return str.c_str(); }

}  // namespace

//
// MinHook
//

MinHook::MinHook(HookSettings* settings, HookDebugger* debugger) : HookObject(settings, debugger) {}

void MinHook::SetUp(Context* ctx) { BIFROST_CHECK_MH(MH_Initialize(), "initialize MinHook"); }

void MinHook::TearDown(Context* ctx) { BIFROST_CHECK_MH(MH_Uninitialize(), "uninitialize MinHook"); }

void MinHook::SetHook(Context* ctx, void* target, void* detour, void** original) {
  BIFROST_HOOK_TRACE(ctx, "MinHook: Creating hook from %s to %s", Sym(ctx, target), Sym(ctx, detour));

  BIFROST_CHECK_MH(MH_CreateHook(target, detour, original), StringFormat("to create hook from %s to %s", Sym(ctx, target), Sym(ctx, detour)));
  BIFROST_CHECK_MH(MH_EnableHook(target), StringFormat("to enable hook from %s", Sym(ctx, target)));

  Debugger().RegisterTrampoline(*original, target);
}

void MinHook::RemoveHook(Context* ctx, void* target) {
  BIFROST_HOOK_TRACE(ctx, "MinHook: Removing hook from %s", Sym(ctx, target));

  BIFROST_CHECK_MH(MH_DisableHook(target), StringFormat("to disable hook from %s", Sym(ctx, target)));
  BIFROST_CHECK_MH(MH_RemoveHook(target), StringFormat("to remmove hook from %s", Sym(ctx, target)));

  Debugger().UnregisterTrampoline(target);
}

bifrost::EHookType MinHook::GetType() const noexcept { return EHookType::E_CFunction; }

//
// VTable
//

VTableHook::VTableHook(HookSettings* settings, HookDebugger* debugger) : HookObject(settings, debugger) {}

void VTableHook::SetUp(Context* ctx) {}

void VTableHook::TearDown(Context* ctx) {}

void VTableHook::SetHook(Context* ctx, void* target, void* detour, void** original) {}

void VTableHook::RemoveHook(Context* ctx, void* target) {}

bifrost::EHookType VTableHook::GetType() const noexcept { return EHookType::E_VTable; }

}  // namespace bifrost