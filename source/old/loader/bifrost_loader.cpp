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

#include "bifrost_loader/common.h"
#include "bifrost_loader/bifrost_loader.h"
#include "bifrost_loader/plugin_loader.h"
#include "bifrost/core/macros.h"
#include "bifrost/core/type.h"
#include "bifrost/core/api_shared.h"
#include "bifrost/core/global_object.h"
#include "bifrost/core/mutex.h"

using namespace bifrost;
using namespace bifrost::loader;

// Serialize all access
static std::mutex g_Mutex;

#define BIFROST_LOADER_CATCH_ALL(stmts) \
  try {                                 \
    stmts                               \
  } catch (std::bad_alloc&) {           \
    return BFL_OUT_OF_MEMORY;           \
  } catch (std::exception&) {           \
    return BFL_UNKNOWN;                 \
  }

namespace {

bfl_Status Attach(HMODULE hModule) {
  BIFROST_LOADER_CATCH_ALL({
    using namespace bifrost::api;
    GlobalObject().Logging().SetModuleName("bifrost_loader.dll");

    i32 pid = Shared::Get().ReadInt(BFL("executable.pid"), -1);
    if (pid == -1) return BFL_OK;
    if (pid != ::GetCurrentProcessId()) return BFL_OK;

    return BFL_OK;
  });
}

bfl_Status Detach(HMODULE hModule) {
  BIFROST_LOADER_CATCH_ALL({
    GlobalObject::Unload();
    return BFL_OK;
  });
}

}  // namespace

BIFROST_LOADER_API const char* bfl_StatusString(bfl_Status status) {
  switch (status) {
    case BFL_OK:
      return "BFL_OK - OK";
    case BFL_OUT_OF_MEMORY:
      return "BFL_OUT_OF_MEMORY - Out of shared memory";
    default:
      return "BFL_UNKNOWN - Unknown error";
  }
}

BIFROST_LOADER_API const char* bfl_GetVersion() {
  return BIFROST_STRINGIFY(BIFROST_LOADER_VERSION_MAJOR) "." BIFROST_STRINGIFY(BIFROST_LOADER_VERSION_MINOR) "." BIFROST_STRINGIFY(
      BIFROST_LOADER_VERSION_PATCH);
}

BIFROST_LOADER_API bfl_Status bfl_Reset() {
  BIFROST_LOCK_GUARD(g_Mutex);
  BIFROST_LOADER_CATCH_ALL({ return PluginLoader::Get().Reset(); });
}

BIFROST_LOADER_API bfl_Status bfl_RegisterPlugin(const bfl_Plugin* plugin) {
  BIFROST_LOCK_GUARD(g_Mutex);
  BIFROST_LOADER_CATCH_ALL({ return PluginLoader::Get().RegisterPlugin(plugin); });
}

BIFROST_LOADER_API bfl_Status bfl_RegisterExecutable(int pid) {
  BIFROST_LOCK_GUARD(g_Mutex);
  BIFROST_LOADER_CATCH_ALL({
    bifrost::api::Shared::Get().WriteInt(BFL("executable.pid"), pid);
    return BFL_OK;
  });
}

extern "C" BOOL APIENTRY DllMain(HMODULE hModule, DWORD ulReasonForCall, LPVOID lpReserved) {
  switch (ulReasonForCall) {
    case DLL_PROCESS_ATTACH:
      return Attach(hModule) == BFL_OK;
    case DLL_PROCESS_DETACH:
      return Detach(hModule) == BFL_OK;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
      break;
  }
  return TRUE;
}
