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

static EHookType GetType(bfp_HookType type) {
  switch (type) {
    case BFP_CFUNCTION:
      return EHookType::E_CFunction;
    case BFP_VTABLE:
      return EHookType::E_VTable;
  }
  return EHookType::E_NumTypes;
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

static bfp_Status HookSetImpl(bfp_PluginContext* ctx, uint32_t num, const bfp_HookSetDesc* descs, bfp_HookSetResult** results) {
  (*results) = nullptr; 

  // Convert the set description
  std::vector<HookManager::SetDesc> setDescs(num);
  for (u32 i = 0; i < num; ++i) setDescs[i] = HookManager::SetDesc{descs[i].Priority, HookTarget{GetType(descs[i].Type), descs[i].Target}, descs[i].Detour};

  // Set the hooks
  auto setResults = g_manager->SetHooks(Get(ctx)->GetContext(), Get(ctx)->GetId(), setDescs.data(), setDescs.size());
  (*results) = new bfp_HookSetResult[setResults.size()];

  // Convert the results
  for (u32 i = 0; i < setResults.size(); ++i) (*results)[i] = bfp_HookSetResult{setResults[i].Original};
  return BFP_OK;
}

bfp_Status bfp_HookSet(bfp_PluginContext* ctx, uint32_t num, const bfp_HookSetDesc* descs, bfp_HookSetResult** results) {
  BIFROST_PLUGIN_CATCH_ALL({ return HookSetImpl(ctx, num, descs, results); });
}

bfp_Status bfp_HookFreeSetResult(bfp_PluginContext* ctx, bfp_HookSetResult* results) {
  BIFROST_PLUGIN_CATCH_ALL({
    if (results) delete[] results;
    return BFP_OK;
  });
}

static bfp_Status HookRemoveImpl(bfp_PluginContext* ctx, uint32_t num, const bfp_HookRemoveDesc* descs) {
  // Convert the remove descriptions
  std::vector<HookManager::RemoveDesc> rmDescs(num);
  for (u32 i = 0; i < num; ++i) rmDescs[i] = HookManager::RemoveDesc{HookTarget{GetType(descs[i].Type), descs[i].Target}};

  // Remove the hooks
  g_manager->RemoveHooks(Get(ctx)->GetContext(), Get(ctx)->GetId(), rmDescs.data(), rmDescs.size());
  return BFP_OK;
}

bfp_Status bfp_HookRemove(bfp_PluginContext* ctx, uint32_t num, const bfp_HookRemoveDesc* descs) {
  BIFROST_PLUGIN_CATCH_ALL({ return HookRemoveImpl(ctx, num, descs); });
}

bfp_Status bfp_HookEnableDebug(bfp_PluginContext* ctx) {
  BIFROST_PLUGIN_CATCH_ALL({
    g_manager->EnableDebug(Get(ctx)->GetContext());
    return BFP_OK;
  });
}

#pragma endregion