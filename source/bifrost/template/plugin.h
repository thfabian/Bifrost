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

#pragma region Namespace

#ifndef BIFROST_NAMESPACE
#define BIFROST_NAMESPACE bifrost
#endif

#define BIFROST_NAMESPACE_BEGIN namespace BIFROST_NAMESPACE {
#define BIFROST_NAMESPACE_END }

#define BIFROST_NAMESPACE_CONCAT_IMPL(x, y) x##y
#define BIFROST_NAMESPACE_CONCAT(x, y) BIFROST_NAMESPACE_CONCAT_IMPL(x, y)

#define BIFROST_NAMESPACE_UNQUALIFIED(x) BIFROST_NAMESPACE_CONCAT(::, BIFROST_NAMESPACE_CONCAT(BIFROST_NAMESPACE, BIFROST_NAMESPACE_CONCAT(::, x)))

#pragma endregion

#pragma region Interface

struct bfp_PluginContext_t;

BIFROST_NAMESPACE_BEGIN

/// Plugin interface to be implemented
///
/// Register your plugin via the macro `BIFROST_REGISTER_PLUGIN`.
class Plugin {
 public:
  /// Access the singleton instance
  static Plugin& Get();

  /// Virtual destructor
  virtual ~Plugin();

  /// Access the singleton instance as `T`
  template <class T>
  static T& GetAs() {
    return *(T*)&Get();
  }

  //
  // IDENTIFIER
  //
  enum class Identifer : std::uint64_t {
    Unsused = 0,
#ifdef BIFROST_CODEGEN
    $BIFROST_PLUGIN_IDENTIFIER$
#elif defined(BIFROST_PLUGIN_TEST)
    saxpy,
#endif
        NumIdentifier
  };

  //
  // INTERFACE
  //

  /// Called when setting up the plugin
  ///
  /// Use this function instead of the constructor.
  virtual void SetUp() = 0;

  /// Called when tearing down the plugin
  ///
  /// Use this function instead of the destructor.
  virtual void TearDown() = 0;

  /// Get the name of the plugin - by default returns the class name of the plugin i.e name passed to `BIFROST_REGISTER_PLUGIN`
  virtual const char* GetName() const;

  /// Handle incoming messages to this plugin - by default does nothing
  ///
  /// @param[in] data  Start of the message
  /// @param[in] size  Size of the message in bytes
  /// @return `true` if the message was handled successfully, `false` otherwise
  virtual bool HandleMessage(const void* data, int sizeInBytes);

  /// Called if a fatal exception occurred - by default logs to Error and throws std::runtime_error
  ///
  /// @param[in] msg   Error message which has to be '\0' terminated
  virtual void FatalError(const char* msg) const;

  //
  // HOOKING
  //

  /// Hook a plain "C" function
  class Hook {
   public:
    /// Get the function pointer to the original function/method
    inline void* Original() const noexcept { return m_original; }

    /// Get the function pointer to the override of the original function/method
    inline void* Override() const noexcept { return m_override; }

    /// Activates the hook, calls `FatalError` if an error occurred.
    void Activate();

    /// Tries to activate the hook, returns `true` on success.
    bool TryActivate();

    /// Deactivate the hook, calls `FatalError` if an error occurred.
    void Deactivate();

    /// Query if this hook is currently active
    inline bool IsActive() const noexcept { return m_isActive; }

    void _SetOverride(void* override) noexcept;
    void _SetOriginal(void* original) noexcept;

   private:
    void* m_original = nullptr;
    void* m_override = nullptr;
    bool m_isActive = false;
  };

  /// Hook the function given by `identifier` to call `override` instead
  ///
  /// If the hook has already been set, the hook is first deactivated and a new Hook object is created.
  /// Calls `FatalError` if an error occurred.
  ///
  /// @param[in] identifier Identifer of the function to override
  /// @param[in] override   Function pointer to use for the override
  /// @param[in] activate   Immediately activate the hook?
  /// @returns a reference to the hook object
  Hook* SetHook(Identifer identifier, void* override, bool activate = true);
  Hook* SetHook(const char* identifier, void* override, bool activate = true);

  /// Get an already set hook
  ///
  /// @param[in] identifier  Identifer of the hook
  /// @returns a reference to the hook object or NULL if the hook has not been set yet.
  Hook* GetHook(Identifer identifier) noexcept;
  Hook* GetHook(const char* identifier);

  /// Check if the `identifier` as a registered hook (the hook may be inactive)
  ///
  /// @param[in] identifier  Identifer of the hook
  /// @returns `true` if a hook has been set, false otherwise.
  bool HasHook(Identifer identifier) noexcept;
  bool HasHook(const char* identifier);

  /// Deactivate all hooks
  void DeactivateAllHooks() noexcept;

  //
  // HELPER
  //

  /// Access the arguments which were passed to the plugin - calls `FatalError` if the arguments couldn't be set properly
  const char* GetArguments() const;

  /// Severity level
  enum class LogLevel : unsigned int { Debug = 0, Info, Warn, Error, Disable };

  /// Log a message - calls Error on failure
  ///
  /// @param[in] level         Severity level
  /// @param[in] msg           Message which has to be '\0' terminated
  /// @param[in] ignoreErrors  Don't call `FatalError` if something goes wrong
  void Log(LogLevel level, const char* msg, bool ignoreErrors = false) const;

  //
  // INTERNAL
  //

  template <Identifer identifer>
  inline constexpr Hook* _GetHook() noexcept {
    return &m_hooks[(std::uint64_t)identifer];
  }

  bfp_PluginContext_t* _GetPlugin();
  void _SetUpImpl(bfp_PluginContext_t*);
  void _TearDownImpl(bool);
  void _SetArguments(const char*);

 private:
  static Plugin* s_instance;
  static const char* s_name;

  Hook m_hooks[(std::uint64_t)Identifer::NumIdentifier];
  bfp_PluginContext_t* m_plugin = nullptr;
  bool m_init = false;
  const char* m_arguments;
};

BIFROST_NAMESPACE_END

/// Define a plugin
#define BIFROST_PLUGIN BIFROST_NAMESPACE_UNQUALIFIED(Plugin)

/// Register a plugin
#define BIFROST_REGISTER_PLUGIN(plugin)       \
  BIFROST_NAMESPACE_BEGIN                     \
  Plugin& Plugin::Get() {                     \
    if (!s_instance) s_instance = new plugin; \
    return *s_instance;                       \
  }                                           \
  const char* Plugin::s_name = #plugin;       \
  BIFROST_NAMESPACE_END

/// Register a help function which returns const char* and has no arguments (e.g `const char* Help()`)
#define BIFROST_REGISTER_PLUGIN_HELP(helpFunc) \
  BIFROST_PLUGIN_HELP_PROC_DECL                \
  BIFROST_PLUGIN_HELP_PROC_DEF { return helpFunc(); }

#pragma endregion

#pragma region Hooking DSL

#define _bf_concat_impl(a, b) a##b
#define _bf_concat(a, b) _bf_concat_impl(a, b)

#define _bf_namespace _bf_concate(BIFROST_NAMESPACE, __)

// Implementation of bf_arg
#define _bf_indirect_expand(m, args) m args

#define _bf_concate_(X, Y) X##Y
#define _bf_concate(X, Y) _bf_concate_(X, Y)

#define _bf_num_args2(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, TOTAL, ...) TOTAL
#define _bf_num_args_(...) _bf_indirect_expand(_bf_num_args2, (__VA_ARGS__))
#define _bf_num_args(...) _bf_num_args_(__VA_ARGS__, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define _bf_var_macro(MACRO, ...) _bf_indirect_expand(_bf_concate, (MACRO, _bf_num_args(__VA_ARGS__)))(__VA_ARGS__)

#define _bf_arg(...) _bf_var_macro(_bf_arg, __VA_ARGS__)
#define _bf_arg_idx(idx) _bf_concat(_bf_concat(_bf_arg_, idx), _bf_concat(_, _bf_concat(_bf_namespace, bf_id)))
#define _bf_arg1(a1) _bf_arg_idx(a1)
#define _bf_arg2(a1, a2) _bf_arg1(a1), _bf_arg_idx(a2)
#define _bf_arg3(a1, a2, a3) _bf_arg2(a1, a2), _bf_arg_idx(a3)
#define _bf_arg4(a1, a2, a3, a4) _bf_arg3(a1, a2, a3), _bf_arg_idx(a4)
#define _bf_arg5(a1, a2, a3, a4, a5) _bf_arg4(a1, a2, a3, a4), _bf_arg_idx(a5)
#define _bf_arg6(a1, a2, a3, a4, a5, a6) _bf_arg5(a1, a2, a3, a4, a5), _bf_arg_idx(a6)
#define _bf_arg7(a1, a2, a3, a4, a5, a6, a7) _bf_arg6(a1, a2, a3, a4, a5, a6), _bf_arg_idx(a7)
#define _bf_arg8(a1, a2, a3, a4, a5, a6, a7, a8) _bf_arg7(a1, a2, a3, a4, a5, a6, a7), _bf_arg_idx(a8)
#define _bf_arg9(a1, a2, a3, a4, a5, a6, a7, a8, a9) _bf_arg8(a1, a2, a3, a4, a5, a6, a7, a8), _bf_arg_idx(a9)
#define _bf_arg10(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) _bf_arg9(a1, a2, a3, a4, a5, a6, a7, a8, a9), _bf_arg_idx(a10)
#define _bf_arg11(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) _bf_arg10(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10), _bf_arg_idx(a11)
#define _bf_arg12(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) _bf_arg11(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11), _bf_arg_idx(a12)

// Implementation of bf_override
#define _bf_func_decl_ret _bf_concat(_bf_func_decl_ret_, _bf_concat(_bf_namespace, bf_id))
#define _bf_func_decl_args _bf_concat(_bf_func_decl_args_, _bf_concat(_bf_namespace, bf_id))

// Implementation of bf_original
#define _bf_original _bf_concat(_bf_func_, _bf_concat(_bf_namespace, bf_id))

// Implementation of bf_args
#define _bf_args _bf_concat(_bf_args_, _bf_concat(_bf_namespace, bf_id))

/// bf_override
#define bf_override(name) _bf_func_decl_ret name(_bf_func_decl_args)

/// bf_original
#define bf_original(...) _bf_original(__VA_ARGS__)

/// bf_args
#define bf_args _bf_args

/// bf_arg
#define bf_arg(...) _bf_arg(__VA_ARGS__)

#define bf_arg_1 _bf_arg(1)
#define bf_arg_2 _bf_arg(2)
#define bf_arg_3 _bf_arg(3)
#define bf_arg_4 _bf_arg(4)
#define bf_arg_5 _bf_arg(5)
#define bf_arg_6 _bf_arg(6)
#define bf_arg_7 _bf_arg(7)
#define bf_arg_8 _bf_arg(8)
#define bf_arg_9 _bf_arg(9)
#define bf_arg_10 _bf_arg(10)
#define bf_arg_11 _bf_arg(11)
#define bf_arg_12 _bf_arg(12)
#define bf_arg_13 _bf_arg(13)
#define bf_arg_14 _bf_arg(14)
#define bf_arg_15 _bf_arg(15)
#define bf_arg_16 _bf_arg(16)
#define bf_arg_17 _bf_arg(17)
#define bf_arg_18 _bf_arg(18)
#define bf_arg_19 _bf_arg(19)
#define bf_arg_20 _bf_arg(20)
#define bf_arg_21 _bf_arg(21)
#define bf_arg_22 _bf_arg(22)
#define bf_arg_23 _bf_arg(23)
#define bf_arg_24 _bf_arg(24)
#define bf_arg_25 _bf_arg(25)
#define bf_arg_26 _bf_arg(26)
#define bf_arg_27 _bf_arg(27)
#define bf_arg_28 _bf_arg(28)
#define bf_arg_29 _bf_arg(29)
#define bf_arg_30 _bf_arg(30)

/// bf_arg_name
#define bf_arg_name(index)

/// bf_arg_type
#define bf_arg_name(index)

#define bf_arg_name(index)
#define bf_arg_name(index)
#define bf_arg_name(index)

/// bf_call_original
#define bf_call_orignal bf_original(bf_args)

#ifdef __INTELLISENSE__
#undef bf_original
#define bf_original(...) _bf_original(bf_args)
#endif

#ifdef BIFROST_CODEGEN
$BIFROST_PLUGIN_DSL_DEF$
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

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <sstream>
#include <unordered_map>

#include "bifrost/api/plugin.h"

BIFROST_NAMESPACE_BEGIN

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

  BIFROST_PLUGIN_API_DECL(bfp_GetVersion)
  BIFROST_PLUGIN_API_DECL(bfp_PluginFree)
  BIFROST_PLUGIN_API_DECL(bfp_PluginGetLastError)
  BIFROST_PLUGIN_API_DECL(bfp_PluginInit)
  BIFROST_PLUGIN_API_DECL(bfp_PluginLog)
  BIFROST_PLUGIN_API_DECL(bfp_PluginSetUpStart)
  BIFROST_PLUGIN_API_DECL(bfp_PluginSetUpEnd)
  BIFROST_PLUGIN_API_DECL(bfp_PluginTearDownStart)
  BIFROST_PLUGIN_API_DECL(bfp_PluginTearDownEnd)

  BifrostPluginApi() {
    HMODULE hModule = NULL;
    Check("LoadLibrary: bifrost_plugin.dll", (hModule = ::LoadLibraryW(L"bifrost_plugin.dll")) != NULL);
    BIFROST_PLUGIN_API_DEF(bfp_GetVersion)
    BIFROST_PLUGIN_API_DEF(bfp_PluginFree)
    BIFROST_PLUGIN_API_DEF(bfp_PluginGetLastError)
    BIFROST_PLUGIN_API_DEF(bfp_PluginInit)
    BIFROST_PLUGIN_API_DEF(bfp_PluginLog)
    BIFROST_PLUGIN_API_DEF(bfp_PluginSetUpStart)
    BIFROST_PLUGIN_API_DEF(bfp_PluginSetUpEnd)
    BIFROST_PLUGIN_API_DEF(bfp_PluginTearDownStart)
    BIFROST_PLUGIN_API_DEF(bfp_PluginTearDownEnd)

#undef BIFROST_PLUGIN_API_DECL
#undef BIFROST_PLUGIN_API_DEF

    // Check the version - this can be critical if we loaded several plugins
    bfp_Version version = this->bfp_GetVersion();
    bool majorVersionOk = version.Major == BIFROST_PLUGIN_VERSION_MAJOR;
    bool minorVersionOk = version.Minor >= BIFROST_PLUGIN_VERSION_MINOR;

    if (!majorVersionOk || !minorVersionOk) {
      std::stringstream ss;
      ss << "The loaded version of bifrost_plugin.dll (" << version.Major << "." << version.Minor << "." << version.Patch
         << ") is incompatible with the requested version (" << BIFROST_PLUGIN_VERSION_MAJOR << "." << BIFROST_PLUGIN_VERSION_MINOR << "."
         << BIFROST_PLUGIN_VERSION_PATCH << "."
         << ")\n";
      if (!majorVersionOk) ss << " > Major versions do not match\n";
      if (!minorVersionOk) ss << " > Minor version of loaded bifrost_plugin.dll is less than requested\n";
      Check(ss.str().c_str(), false);
    }
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

static Plugin::Identifer GetIdentifier(const char* identifer) {
  static std::unordered_map<std::string, Plugin::Identifer> map{
#ifdef BIFROST_CODEGEN
      $BIFROST_PLUGIN_IDENTIFIER_MAP$
#elif defined(BIFROST_PLUGIN_TEST)
      {"saxpy", Plugin::Identifer::saxpy},
#endif
  };

  auto it = map.find(identifer);
  if (it != map.end()) {
    std::stringstream ss;
    ss << "Identifier \"" << identifer << "\" does not exist";
    Plugin::Get().FatalError(ss.str().c_str());
  }
  return it->second;
}

}  // namespace

Plugin::~Plugin() {
  DeactivateAllHooks();
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

void Plugin::Hook::Activate() {
  if (IsActive()) return;
  if (!TryActivate()) Plugin::Get().FatalError("Failed to hook function ");
}

bool Plugin::Hook::TryActivate() {
#ifdef BIFROST_DEBUG
  Plugin::Get().Log(LogLevel::Debug, "Setting up hook for ...");
#endif
  m_isActive = true;
  return true;
}

void Plugin::Hook::Deactivate() {
  if (!IsActive()) return;
#ifdef BIFROST_DEBUG
  Plugin::Get().Log(LogLevel::Debug, "Removing hook from ...");
#endif
}

void Plugin::Hook::_SetOverride(void* override) noexcept { m_override = override; }
void Plugin::Hook::_SetOriginal(void* original) noexcept { m_original = original; }

Plugin::Hook* Plugin::SetHook(const char* identifier, void* override, bool activate) { return SetHook(GetIdentifier(identifier), override, activate); }

Plugin::Hook* Plugin::SetHook(Plugin::Identifer identifier, void* override, bool activate) { return nullptr; }

Plugin::Hook* Plugin::GetHook(Plugin::Identifer identifier) noexcept { return nullptr; }

Plugin::Hook* Plugin::GetHook(const char* identifier) { return GetHook(GetIdentifier(identifier)); }

bool Plugin::HasHook(Plugin::Identifer identifier) noexcept { return GetHook(identifier) != nullptr; }

bool Plugin::HasHook(const char* identifier) { return HasHook(GetIdentifier(identifier)); }

void Plugin::DeactivateAllHooks() noexcept {
  for (std::uint64_t i = 0; i < (std::uint64_t)Identifer::NumIdentifier; ++i) {
    if (m_hooks[i].Override() != nullptr) {
      try {
        m_hooks[i].Deactivate();
      } catch (...) {
      }
    }
  }
}

const char* Plugin::GetName() const { return s_name; }

bool Plugin::HandleMessage(const void* data, int sizeInBytes) { return true; }

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
  DeactivateAllHooks();
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

BIFROST_NAMESPACE_END

#include "bifrost/template/plugin_fwd.h"

BIFROST_PLUGIN_SETUP_PROC_DECL

BIFROST_PLUGIN_SETUP_PROC_DEF {
  using namespace BIFROST_NAMESPACE;
  auto& api = GetApi();

  Plugin* plugin = &Plugin::Get();
  bfp_PluginContext* ctx = api.bfp_PluginInit();

  bfp_PluginSetUpArguments* args;
  if (api.bfp_PluginSetUpStart(ctx, plugin->GetName(), param, &args) != BFP_OK) throw std::runtime_error(api.bfp_PluginGetLastError(ctx));

  plugin->_SetArguments(args->Arguments);
  plugin->_SetUpImpl((bfp_PluginContext_t*)ctx);

  if (api.bfp_PluginSetUpEnd(ctx, plugin->GetName(), param, args) != BFP_OK) throw std::runtime_error(api.bfp_PluginGetLastError(ctx));
  return 0;
}

BIFROST_PLUGIN_TEARDOWN_PROC_DECL

BIFROST_PLUGIN_TEARDOWN_PROC_DEF {
  using namespace BIFROST_NAMESPACE;
  auto& api = GetApi();

  Plugin* plugin = &Plugin::Get();
  bfp_PluginContext* ctx = (bfp_PluginContext*)plugin->_GetPlugin();

  bfp_PluginTearDownArguments* args;
  if (api.bfp_PluginTearDownStart(ctx, param, &args) != BFP_OK) throw std::runtime_error(api.bfp_PluginGetLastError(ctx));

  plugin->_TearDownImpl(args->NoFail);

  if (api.bfp_PluginTearDownEnd(ctx, param, args) != BFP_OK) throw std::runtime_error(api.bfp_PluginGetLastError(ctx));

  api.bfp_PluginFree(ctx);
  return 0;
}

BIFROST_PLUGIN_MESSAGE_PROC_DECL

BIFROST_PLUGIN_MESSAGE_PROC_DEF {
  using namespace BIFROST_NAMESPACE;

  Plugin* plugin = &Plugin::Get();
  return plugin->HandleMessage(data, sizeInBytes) ? 1 : 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) { return TRUE; }

#ifdef BIFROST_CODEGEN
$BIFROST_PLUGIN_IMPLEMENTATION$
#endif

#endif

#pragma endregion
