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

#pragma once

#pragma region Includes

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "bifrost/api/plugin.h"

#include "bifrost/template/plugin_decl.h"

#pragma endregion

#ifdef BIFROST_PLUGIN
$BIFROST_PLUGIN_HEADER$
#endif

#pragma region Implementation
#if defined(BIFROST_IMPLEMENTATION) || defined(__INTELLISENSE__)

#include <Windows.h>

#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <filesystem>

#pragma region Bifrost Plugin Dll

namespace bifrost {

Plugin* Plugin::s_instance = nullptr;

namespace {

/// Function used to determine the name of this DLL
static bool FunctionInThisDll() { return true; }

/// Get the most recent Win32 error
std::string GetLastWin32Error() {
  DWORD errorCode = ::GetLastError();
  if (errorCode == 0) return "Unknown Error.\n";

  LPSTR messageBuffer = nullptr;
  size_t size = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode,
                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

  std::string message(messageBuffer, size);
  ::LocalFree(messageBuffer);
  return message;
}

/// Interaction with bifrost_plugin.dll
class BifrostPluginApi {
 public:
  using bfp_PluginInit_fn = decltype(&bfp_PluginInit);
  bfp_PluginInit_fn bfp_PluginInit;

  using bfp_PluginSetUp_fn = decltype(&bfp_PluginSetUp);
  bfp_PluginSetUp_fn bfp_PluginSetUp;

  using bfp_PluginGetLastError_fn = decltype(&bfp_PluginGetLastError);
  bfp_PluginGetLastError_fn bfp_PluginGetLastError;

  BifrostPluginApi() {
    HMODULE hModule = NULL;
    Check("LoadLibrary: bifrost_plugin.dll", (hModule = ::LoadLibraryW(L"bifrost_plugin.dll")) != NULL);
    Check("GetProcAddress: bfp_PluginInit", (bfp_PluginInit = (bfp_PluginInit_fn)::GetProcAddress(hModule, "bfp_PluginInit")) != NULL);
    Check("GetProcAddress: bfp_PluginSetUp", (bfp_PluginSetUp = (bfp_PluginSetUp_fn)::GetProcAddress(hModule, "bfp_PluginSetUp")) != NULL);
    Check("GetProcAddress: bfp_PluginGetLastError",
          (bfp_PluginGetLastError = (bfp_PluginGetLastError_fn)::GetProcAddress(hModule, "bfp_PluginGetLastError")) != NULL);
  }

 private:
  void Check(const char* msg, bool success) {
    if (!success) {
      auto errMsg = std::string(msg) + ": " + GetLastWin32Error();

      // Write to stderr
      std::cerr << "[ERROR] " << errMsg << std::endl;

      // Write to file
      std::string path(MAX_PATH, '\0');
      bool queriedDllName = false;

      HMODULE hModule = NULL;
      if (::GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&FunctionInThisDll, &hModule) !=
          0) {
        DWORD err = ERROR_SUCCESS;
        do {
          path.resize(path.size() * 2);
          err = ::GetModuleFileNameA(hModule, (LPSTR)path.c_str(), (DWORD)path.size()) != 0;
        } while (err == ERROR_INSUFFICIENT_BUFFER);

        if (err == ERROR_SUCCESS) {
          auto idx = path.find_last_of("\\/");
          if (idx != -1) path = path.substr(idx);
          queriedDllName = true;
        }
      }
      if (!queriedDllName) path = "plugin";
      std::ofstream ofs("log." + path + ".txt");
      ofs << errMsg << std::endl;
    }
  }
};
static BifrostPluginApi* g_api = nullptr;
static std::mutex g_mutex;

/// Free the API singleton (thread-safe)
static void FreeApi() {
  std::lock_guard<std::mutex> lock(g_mutex);
  if (g_api) {
    delete g_api;
    g_api = nullptr;
  }
}

/// Access the API singleton (thread-safe)
static BifrostPluginApi& GetApi() {
  if (!g_api) {
    std::lock_guard<std::mutex> lock(g_mutex);
    if (!g_api) {
      g_api = new BifrostPluginApi;
      std::atexit(FreeApi);
    }
  }
  return *g_api;
}

#pragma endregion

}  // namespace

}  // namespace bifrost

BIFROST_PLUGIN_SETUP_PROC_DECL

BIFROST_PLUGIN_SETUP_PROC_DEF {
  using namespace bifrost;
  auto plugin = GetApi().bfp_PluginInit();
  if (GetApi().bfp_PluginSetUp(plugin, param) != BFI_OK) throw std::runtime_error(GetApi().bfp_PluginGetLastError(plugin));
  return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) { return TRUE; }

#ifdef BIFROST_PLUGIN
$BIFROST_PLUGIN_IMPLEMENTATION$
#endif

#endif

#pragma endregion
