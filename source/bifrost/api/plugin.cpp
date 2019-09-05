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
#include "bifrost/core/macros.h"
#include "bifrost/core/mutex.h"

#include "MinHook.h"

using namespace bifrost;
using namespace bifrost::api;

namespace {

#define BIFROST_PLUGIN_CATCH_ALL(stmts) BIFROST_API_CATCH_ALL_IMPL(ctx, stmts, BFP_ERROR)
#define BIFROST_PLUGIN_CATCH_ALL_PTR(stmts) BIFROST_API_CATCH_ALL_IMPL(ctx, stmts, nullptr)
#define BIFROST_PLUGIN_CHECK_MH(ret)                \
  if (ret != MH_OK) {                               \
    Get(ctx)->SetLastError(MH_StatusToString(ret)); \
    return BFP_ERROR;                               \
  } else {                                          \
    return BFP_OK;                                  \
  }

PluginContext* Get(bfp_PluginContext* ctx) { return (PluginContext*)ctx->_Internal; }

static std::mutex g_mutex;
static int g_MinHookRef = 0;

}  // namespace

#pragma region Version

bfp_Version bfp_GetVersion(void) { return {BIFROST_PLUGIN_VERSION_MAJOR, BIFROST_PLUGIN_VERSION_MINOR, BIFROST_PLUGIN_VERSION_PATCH}; }

const char* bfp_GetVersionString(void) {
  return BIFROST_STRINGIFY(BIFROST_PLUGIN_VERSION_MAJOR) "." BIFROST_STRINGIFY(BIFROST_PLUGIN_VERSION_MINOR) "." BIFROST_STRINGIFY(
      BIFROST_PLUGIN_VERSION_PATCH);
}

#pragma endregion

#pragma region Plugin

bfp_PluginContext* bfp_PluginInit(int32_t* minHookInitSuccess) {
  bfp_PluginContext* ctx = Init<bfp_PluginContext, PluginContext>();

  BIFROST_LOCK_GUARD(g_mutex);
  *minHookInitSuccess = 1;

  if (g_MinHookRef++ == 0) {
    auto status = MH_Initialize();
    if (status != MH_OK) {
      Get(ctx)->SetLastError(::MH_StatusToString(status));
      *minHookInitSuccess = 0;
    }
  }
  return ctx;
}

void bfp_PluginFree(bfp_PluginContext* ctx, int32_t* minHookFreeSuccess) {
  Free<bfp_PluginContext, PluginContext>(ctx);

  BIFROST_LOCK_GUARD(g_mutex);
  *minHookFreeSuccess = 1;

  if (--g_MinHookRef == 0) {
    auto status = MH_Uninitialize();
    if (status != MH_OK) {
      *minHookFreeSuccess = 0;
    }
  }
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

BIFROST_PLUGIN_API bfp_Status bfp_HookCreate(bfp_PluginContext* ctx, void* target, void* detour, uint32_t enable, void** original) { return BFP_OK; }

BIFROST_PLUGIN_API bfp_Status bfp_HookRemove(bfp_PluginContext* ctx, void* target) { return BFP_OK; }

BIFROST_PLUGIN_API bfp_Status bfp_HookEnable(bfp_PluginContext* ctx, void* target) { return BFP_OK; }

BIFROST_PLUGIN_API bfp_Status bfp_HookDisable(bfp_PluginContext* ctx, void* target) { return BFP_OK; }

#pragma endregion