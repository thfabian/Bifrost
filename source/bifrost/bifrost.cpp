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

#include "bifrost/bifrost.h"
#include "bifrost_core/error.h"
#include "bifrost_core/macros.h"
#include "bifrost_core/module_loader.h"
#include "bifrost_core/logging.h"

using namespace bifrost;

#pragma region Error

extern BIFROST_API const char* bf_GetLastError() { return Error::Get().GetLastError(); }

#pragma endregion

#pragma region Version

extern BIFROST_API const char* bf_GetVersion() {
  return BIFROST_STRINGIFY(BIFROST_VERSION_MAJOR) "." BIFROST_STRINGIFY(BIFROST_VERSION_MINOR) "." BIFROST_STRINGIFY(BIFROST_VERSION_PATCH);
}

#pragma endregion

#pragma region Logging

extern BIFROST_API bf_Status bf_RegisterLogCallback(const char* name, bf_LogCallback_t cb) {
  Logging::Get().SetCallback(name, cb);
  return BF_OK;
}

extern BIFROST_API bf_Status bf_UnregisterLogCallback(const char* name) {
  Logging::Get().RemoveCallback(name);
  return BF_OK;
}

extern BIFROST_API bf_Status bf_Log(int level, const char* message) {
  switch (level) {
    case BF_LOGLEVEL_DEBUG:
      BIFROST_LOG_DEBUG(message);
      break;
    case BF_LOGLEVEL_INFO:
      BIFROST_LOG_INFO(message);
      break;
    case BF_LOGLEVEL_WARN:
      BIFROST_LOG_WARN(message);
      break;
    case BF_LOGLEVEL_ERROR:
      BIFROST_LOG_ERROR(message);
      break;
    case BF_LOGLEVEL_DISABLE:
    default:
      break;
  }
  return BF_OK;
}

#pragma endregion

#pragma region Plugin

extern BIFROST_API bf_Status bf_RegisterPlugin(const char* name, const char* moduleName, const char* modulePath) {
  // ModuleLoader::Get().GetCurrentModule().ModulePath

  /*
  try {
    SharedConfiguration config(ModuleLoader::Get().GetModule("bifrost_shared"));
    config.Set("__bifrost.plugin." + std::string(name), moduleName);
  } except(std::runtime_error& e) {
    Error::Get().SetLastError("Failed to register plugin '" + std::string(moduleName) + "': " + std::string(e.what()));
  }
  */

  if (modulePath) {
    BIFROST_ASSERT_WIN_CALL_MSG(::SetDllDirectoryA(modulePath) != TRUE, "Failed to set DLL module search path");
  }
  BIFROST_CHECK_WIN_CALL_MSG(::SetDllDirectoryA(NULL) != TRUE, "Failed to restore DLL module search path");

  return BF_OK;
}

#pragma endregion

#pragma region DllMain

extern "C" BOOL APIENTRY DllMain(HMODULE hModule, DWORD ulReasonForCall, LPVOID lpReserved) {
  // 1) load bifrost_shared.sll
  // 2) Get the plugins to load

  switch (ulReasonForCall) {
    case DLL_PROCESS_ATTACH:
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
      break;
  }
  return TRUE;
}

#pragma endregion
