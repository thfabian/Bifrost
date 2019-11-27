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

#include "bifrost/core/hook_cfunction.h"
#include "bifrost/core/hook_debugger.h"
#include "bifrost/core/context.h"
#include "bifrost/core/ilogger.h"
#include "bifrost/core/error.h"

#include "MinHook.h"

namespace bifrost {

namespace {

static const char *GetConstCharPtr(const char *str) { return str; }
static const char *GetConstCharPtr(const std::string &str) { return str.c_str(); }

}  // namespace

#define BIFROST_CHECK_MH(call, reason)                                                                  \
  if (MH_STATUS status; (status = call) != MH_OK) {                                                     \
    throw Exception("MinHook: Failed to %s: %s", GetConstCharPtr(reason), ::MH_StatusToString(status)); \
  }

CFunctionHook::CFunctionHook(HookSettings *settings, HookDebugger *debugger) : HookObject(settings, debugger) {}

void CFunctionHook::SetUp(Context *ctx) { BIFROST_CHECK_MH(MH_Initialize(), "initialize MinHook"); }

void CFunctionHook::TearDown(Context *ctx) { BIFROST_CHECK_MH(MH_Uninitialize(), "uninitialize MinHook"); }

void CFunctionHook::SetHook(Context *ctx, const HookTarget &target, void *detour, void **original) {
  BIFROST_ASSERT(target.Type == EHookType::E_CFunction);
  BIFROST_HOOK_TRACE(ctx, "MinHook: Creating hook from %s to %s", Sym(ctx, target), Sym(ctx, detour));

  BIFROST_CHECK_MH(MH_CreateHook(target.Target, detour, original), StringFormat("to create hook from %s to %s", Sym(ctx, target), Sym(ctx, detour)));
  BIFROST_CHECK_MH(MH_EnableHook(target.Target), StringFormat("to enable hook from %s", Sym(ctx, target)));

  Debugger().RegisterTrampoline(*original, target.Target);
}

void CFunctionHook::RemoveHook(Context *ctx, const HookTarget &target) {
  BIFROST_ASSERT(target.Type == EHookType::E_CFunction);
  BIFROST_HOOK_TRACE(ctx, "MinHook: Removing hook from %s", Sym(ctx, target));

  BIFROST_CHECK_MH(MH_DisableHook(target.Target), StringFormat("to disable hook from %s", Sym(ctx, target)));
  BIFROST_CHECK_MH(MH_RemoveHook(target.Target), StringFormat("to remmove hook from %s", Sym(ctx, target)));

  Debugger().UnregisterTrampoline(target.Target);
}

EHookType CFunctionHook::GetType() const noexcept { return EHookType::E_CFunction; }

}  // namespace bifrost