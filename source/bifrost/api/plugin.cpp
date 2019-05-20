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
#include "bifrost/api/plugin_impl.h"

using namespace bifrost;
using namespace bifrost::api;

namespace {

#define BIFROST_PLUGIN_CATCH_ALL(stmts) BIFROST_API_CATCH_ALL_IMPL(plugin, stmts, BFI_ERROR)
#define BIFROST_PLUGIN_CATCH_ALL_PTR(stmts) BIFROST_API_CATCH_ALL_IMPL(plugin, stmts, nullptr)

Plugin* Get(bfp_Plugin* plugin) { return (Plugin*)plugin->_Internal; }

}  // namespace

#pragma region Version

bfp_Version bfp_GetVersion(void) { return {BIFROST_PLUGIN_VERSION_MAJOR, BIFROST_PLUGIN_VERSION_MINOR, BIFROST_PLUGIN_VERSION_PATCH}; }

const char* bfp_GetVersionString(void) {
  return BIFROST_STRINGIFY(BIFROST_PLUGIN_VERSION_MAJOR) "." BIFROST_STRINGIFY(BIFROST_PLUGIN_VERSION_MINOR) "." BIFROST_STRINGIFY(
      BIFROST_PLUGIN_VERSION_PATCH);
}

#pragma endregion

#pragma region Plugin

bfp_Plugin* bfp_PluginInit(void) { return Init<bfp_Plugin, Plugin>(); }

bfp_Status bfp_PluginSetUp(bfp_Plugin* plugin, void* param) {
  BIFROST_PLUGIN_CATCH_ALL({ return BFI_OK; });
}

bfp_Status bfp_PluginTearDown(bfp_Plugin* plugin) {
  BIFROST_PLUGIN_CATCH_ALL({ return BFI_OK; });
}

void bfp_PluginFree(bfp_Plugin* plugin) { Free<bfp_Plugin, Plugin>(plugin); }

const char* bfi_PluginGetLastError(bfp_Plugin* plugin) { return Get(plugin)->GetLastError(); }

#pragma endregion