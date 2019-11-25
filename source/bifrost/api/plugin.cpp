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

#include "bifrost/api/helper.h"
#include "bifrost/api/plugin_context.h"
#include "bifrost/core/hook_manager.h"

#include "bifrost/core/macros.h"
#include "bifrost/core/mutex.h"
#include "bifrost/core/error.h"

using namespace bifrost;
using namespace bifrost::api;

namespace {

#define BIFROST_PLUGIN_CATCH_ALL(stmts) BIFROST_API_CATCH_ALL_IMPL(ctx, stmts, BFP_ERROR)
#define BIFROST_PLUGIN_CATCH_ALL_PTR(stmts) BIFROST_API_CATCH_ALL_IMPL(ctx, stmts, nullptr)

inline PluginContext* Get(bfp_PluginContext* ctx) { return (PluginContext*)ctx->_Internal; }

// Singleton context
static SpinMutex g_mutex;
static u32 g_refCount = 0;
static HookManager* g_manager = nullptr;

static void SetUpHookManger(PluginContext* ctx) {
  BIFROST_LOCK_GUARD(g_mutex);
  if (!g_manager) {
    g_manager = new HookManager;
    try {
      g_manager->SetUp(ctx->GetContext());
    } catch (...) {
      delete g_manager;
      g_manager = nullptr;
      throw;
    }
  }
  g_refCount++;
}

static void TearDownHookManager(PluginContext* ctx) {
  BIFROST_LOCK_GUARD(g_mutex);
  if (--g_refCount == 0) {
    try {
      g_manager->TearDown(ctx->GetContext());
    } catch (...) {
      delete g_manager;
      g_manager = nullptr;
      throw;
    }
    delete g_manager;
    g_manager = nullptr;
  }
}

template <class DescT>
static HookTarget MakeHookTarget(DescT* desc) {
  HookTarget target;
  switch (desc->Type) {
    case BFP_CFUNCTION:
      target.Type = EHookType::E_CFunction;
      target.CFunction.Target = desc->Target;
      break;
    case BFP_VTABLE:
      target.Type = EHookType::E_VTable;
      target.VTable.Table = desc->Target;
      target.VTable.Offset = desc->TargetOffset;
      break;
  }
  return target;
}

}  // namespace

#pragma region Version

bfp_Version bfp_GetVersion(void) { return {BIFROST_PLUGIN_VERSION_MAJOR, BIFROST_PLUGIN_VERSION_MINOR, BIFROST_PLUGIN_VERSION_PATCH}; }

const char* bfp_GetVersionString(void) {
  return BIFROST_STRINGIFY(BIFROST_PLUGIN_VERSION_MAJOR) "." BIFROST_STRINGIFY(BIFROST_PLUGIN_VERSION_MINOR) "." BIFROST_STRINGIFY(
      BIFROST_PLUGIN_VERSION_PATCH);
}

#pragma endregion

#pragma region Plugin

bfp_PluginContext* bfp_PluginInit(const char** errMsg) {
  bfp_PluginContext* ctx = Init<bfp_PluginContext, PluginContext>();

  try {
    (*errMsg) = nullptr;
    SetUpHookManger(Get(ctx));
    Get(ctx)->SetId(g_manager->MakeUniqueId());
  } catch (std::exception& e) {
    (*errMsg) = StringCopy(StringFormat("Failed to initialize hooking maanger: %s", e.what())).release();
    Free<bfp_PluginContext, PluginContext>(ctx);
  }

  return ctx;
}

void bfp_PluginFree(bfp_PluginContext* ctx, const char** errMsg) {
  try {
    (*errMsg) = nullptr;
    TearDownHookManager(Get(ctx));
    Free<bfp_PluginContext, PluginContext>(ctx);
  } catch (std::exception& e) {
    (*errMsg) = StringCopy(StringFormat("Failed to uninitialize hooking maanger: %s", e.what())).release();
    Free<bfp_PluginContext, PluginContext>(ctx);
  }
}

void bfp_StringFree(const char* str) {
  if (str) delete str;
}

bfp_Status bfp_PluginSetUpStart(bfp_PluginContext* ctx, const char* name, const void* param, bfp_PluginSetUpArguments** args) {
  BIFROST_PLUGIN_CATCH_ALL({ return Get(ctx)->SetUpStart(ctx, name, param, args); });
}

bfp_Status bfp_PluginSetUpEnd(bfp_PluginContext* ctx, const char* name, const void* param, const bfp_PluginSetUpArguments* args) {
  BIFROST_PLUGIN_CATCH_ALL({ return Get(ctx)->SetUpEnd(ctx, name, param, args); });
}

bfp_Status bfp_PluginTearDownStart(bfp_PluginContext* ctx, const void* param, bfp_PluginTearDownArguments** args) {
  BIFROST_PLUGIN_CATCH_ALL({ return Get(ctx)->TearDownStart(ctx, param, args); });
}

bfp_Status bfp_PluginTearDownEnd(bfp_PluginContext* ctx, const void* param, const bfp_PluginTearDownArguments* args) {
  BIFROST_PLUGIN_CATCH_ALL({ return Get(ctx)->TearDownEnd(ctx, param, args); });
}

bfp_Status bfp_PluginLog(bfp_PluginContext* ctx, uint32_t level, const char* module, const char* msg) {
  BIFROST_PLUGIN_CATCH_ALL({ return Get(ctx)->Log(level, module, msg); });
}

const char* bfp_PluginGetLastError(bfp_PluginContext* ctx) { return Get(ctx)->GetLastError(); }

BIFROST_PLUGIN_API bfp_Status bfp_HookSet(bfp_PluginContext* ctx, const bfp_HookSetDesc* desc, void** original) {
  BIFROST_PLUGIN_CATCH_ALL({
    g_manager->SetHook(Get(ctx)->GetContext(), Get(ctx)->GetId(), desc->Priority, MakeHookTarget(desc), desc->Detour, original);
    return BFP_OK;
  });
}

bfp_Status bfp_HookRemove(bfp_PluginContext* ctx, const bfp_HookRemoveDesc* desc) {
  BIFROST_PLUGIN_CATCH_ALL({
    g_manager->RemoveHook(Get(ctx)->GetContext(), Get(ctx)->GetId(), MakeHookTarget(desc));
    return BFP_OK;
  });
}

bfp_Status bfp_HookEnableDebug(bfp_PluginContext* ctx) {
  BIFROST_PLUGIN_CATCH_ALL({
    g_manager->EnableDebug(Get(ctx)->GetContext());
    return BFP_OK;
  });
}

#pragma endregion