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

#include "bifrost/api/plugin_impl.h"

using namespace bifrost;
using namespace bifrost::api;

namespace {

#define BIFROST_PLUGIN_CATCH_ALL(stmts) BIFROST_API_CATCH_ALL_IMPL(ctx, stmts, BFI_ERROR)
#define BIFROST_PLUGIN_CATCH_ALL_PTR(stmts) BIFROST_API_CATCH_ALL_IMPL(ctx, stmts, nullptr)

PluginContext* Get(bfp_PluginContext* ctx) { return (PluginContext*)ctx->_Internal; }

}  // namespace

#pragma region Version

bfp_Version bfp_GetVersion(void) { return {BIFROST_PLUGIN_VERSION_MAJOR, BIFROST_PLUGIN_VERSION_MINOR, BIFROST_PLUGIN_VERSION_PATCH}; }

const char* bfp_GetVersionString(void) {
  return BIFROST_STRINGIFY(BIFROST_PLUGIN_VERSION_MAJOR) "." BIFROST_STRINGIFY(BIFROST_PLUGIN_VERSION_MINOR) "." BIFROST_STRINGIFY(
      BIFROST_PLUGIN_VERSION_PATCH);
}

#pragma endregion

#pragma region Plugin

bfp_PluginContext* bfp_PluginInit(void) { return Init<bfp_PluginContext, PluginContext>(); }

bfp_Status bfp_PluginSetUp(bfp_PluginContext* ctx, const char* name, void* plugin, void* param) {
  BIFROST_PLUGIN_CATCH_ALL({ return Get(ctx)->SetUp(ctx, name, plugin, param); });
}

bfp_Status bfp_PluginTearDown(bfp_PluginContext* ctx, void* plugin, void* param) {
  BIFROST_PLUGIN_CATCH_ALL({ return Get(ctx)->TearDown(ctx, plugin, param); });
}

void bfp_PluginFree(bfp_PluginContext* ctx) { Free<bfp_PluginContext, PluginContext>(ctx); }

const char* bfp_PluginGetLastError(bfp_PluginContext* ctx) { return Get(ctx)->GetLastError(); }

#pragma endregion