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

#include <cstdint>

#include "bifrost/template/plugin_dsl.h"
#include "bifrost/template/plugin_decl.h"

#ifdef BIFROST_PLUGIN
$BIFROST_PLUGIN_HEADER$
#endif


#pragma region Implementation
#if defined(BIFROST_IMPLEMENTATION) || defined(__INTELLISENSE__)

#include <stdexcept>
#include <string>
#include <cstdlib>

namespace bifrost {

void Plugin::_SetUpImpl(bfp_PluginContext_t* plugin) {
  m_plugin = plugin;
  if (m_init) throw std::runtime_error("Plugin already set up");
  SetUp();
  m_init = true;
}

void Plugin::_TearDownImpl(bool noFail) {
  if (noFail && !m_init) return;
  if (!m_init) throw std::runtime_error("Plugin not set up");
  TearDown();
  m_init = false;
}

void Plugin::_SetArguments(const char* arguments) {
  std::string str(arguments);

  try {
    m_arguments = new char[str.size() + 1];
    std::memcpy((void*)m_arguments, str.c_str(), str.size() + 1);
  } catch (...) {
    m_arguments = nullptr;
  }
}

}  // namespace bifrost

#endif
#pragma endregion

#pragma region Implementation
#if defined(BIFROST_IMPLEMENTATION) || defined(__INTELLISENSE__)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

#include <fstream>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>

#include "bifrost/api/plugin.h"

namespace bifrost {

#pragma region Bifrost Plugin Dll

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
#define BIFROST_PLUGIN_API_DECL(name) \
  using name##_fn = decltype(&name);  \
  name##_fn name;
#define BIFROST_PLUGIN_API_DEF(name) Check("GetProcAddress: " #name, (name = (name##_fn)::GetProcAddress(hModule, #name)) != NULL);

  BIFROST_PLUGIN_API_DECL(bfp_PluginFree)
  BIFROST_PLUGIN_API_DECL(bfp_PluginGetLastError)
  BIFROST_PLUGIN_API_DECL(bfp_PluginInit)
  BIFROST_PLUGIN_API_DECL(bfp_PluginLog)
  BIFROST_PLUGIN_API_DECL(bfp_PluginSetUp)
  BIFROST_PLUGIN_API_DECL(bfp_PluginTearDown)

  BifrostPluginApi() {
    HMODULE hModule = NULL;
    Check("LoadLibrary: bifrost_plugin.dll", (hModule = ::LoadLibraryW(L"bifrost_plugin.dll")) != NULL);
    BIFROST_PLUGIN_API_DEF(bfp_PluginFree)
    BIFROST_PLUGIN_API_DEF(bfp_PluginGetLastError)
    BIFROST_PLUGIN_API_DEF(bfp_PluginInit)
    BIFROST_PLUGIN_API_DEF(bfp_PluginLog)
    BIFROST_PLUGIN_API_DEF(bfp_PluginSetUp)
    BIFROST_PLUGIN_API_DEF(bfp_PluginTearDown)

#undef BIFROST_PLUGIN_API_DECL
#undef BIFROST_PLUGIN_API_DEF
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
        DWORD size = 0;
        do {
          path.resize(path.size() * 2);
          size = ::GetModuleFileNameA(hModule, (LPSTR)path.c_str(), (DWORD)path.size());
        } while (size == ERROR_INSUFFICIENT_BUFFER);

        if (size > 0) {
          auto idx = path.find_last_of("\\/");
          if (idx != -1) path = path.substr(idx + 1, size - idx - 1);
          queriedDllName = true;
        }
      }

      if (!queriedDllName) path = "plugin";

      std::string filename = "log." + path + ".txt";
      std::ofstream ofs(filename);
      ofs << errMsg << std::endl;

      throw std::runtime_error(errMsg.c_str());
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

#pragma region Plugin Interface

Plugin::~Plugin() {
  if (m_arguments) delete m_arguments;
}

Plugin* Plugin::s_instance = nullptr;

bfp_PluginContext_t* Plugin::_GetPlugin() { return m_plugin; }

void Plugin::Log(Plugin::LogLevel level, const char* msg, bool ignoreErrors) const {
  auto& api = GetApi();
  if (api.bfp_PluginLog((bfp_PluginContext*)m_plugin, (uint32_t)level, GetName(), msg) != BFP_OK) {
    if (!ignoreErrors) {
      FatalError(api.bfp_PluginGetLastError((bfp_PluginContext*)m_plugin));
    }
  }
}

const char* Plugin::GetArguments() const {
  if (!m_arguments) FatalError("Failed to set arguments during startup");
  return m_arguments;
}

void Plugin::FatalError(const char* msg) const {
  Log(LogLevel::Error, msg, true);
  throw std::runtime_error(msg);
}

const char* Plugin::GetName() const { return s_name; }

bool Plugin::HandleMessage(const void* data, int sizeInBytes) { return true; }

#pragma endregion

}  // namespace bifrost

BIFROST_PLUGIN_SETUP_PROC_DECL

BIFROST_PLUGIN_SETUP_PROC_DEF {
  using namespace bifrost;
  auto& api = GetApi();

  Plugin* plugin = &Plugin::Get();
  bfp_PluginContext* ctx = api.bfp_PluginInit();

  if (api.bfp_PluginSetUp(ctx, plugin->GetName(), (void*)plugin, param) != BFP_OK) {
    throw std::runtime_error(api.bfp_PluginGetLastError(ctx));
  }
  return 0;
}

BIFROST_PLUGIN_TEARDOWN_PROC_DECL

BIFROST_PLUGIN_TEARDOWN_PROC_DEF {
  using namespace bifrost;
  auto& api = GetApi();

  Plugin* plugin = &Plugin::Get();
  bfp_PluginContext* ctx = (bfp_PluginContext*)plugin->_GetPlugin();

  if (api.bfp_PluginTearDown(ctx, (void*)plugin, param) != BFP_OK) {
    throw std::runtime_error(api.bfp_PluginGetLastError(ctx));
  }

  api.bfp_PluginFree(ctx);
  return 0;
}

BIFROST_PLUGIN_MESSAGE_PROC_DECL

BIFROST_PLUGIN_MESSAGE_PROC_DEF {
  using namespace bifrost;

  Plugin* plugin = &Plugin::Get();
  return plugin->HandleMessage(data, sizeInBytes) ? 1 : 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) { return TRUE; }

#ifdef BIFROST_PLUGIN
$BIFROST_PLUGIN_IMPLEMENTATION$
#endif

#endif

#pragma endregion
