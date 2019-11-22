// The following macros can be defined to alter the implementation:
//
//		BIFROST_IMPLEMENTATION	    			Include the implementation, has to be defined exactly once per application/plugin.
//		BIFROST_DEBUG						<int>			Set to 1 to enable debug mode (default: 0).
//		BIFROST_NO_INCLUDE			<int>			Set to 1 to disable the include if the original headers (default: 0)
//		BIFROST_CACHE_LINE			<int>			Size of a cache-line (default: 64).
//

// https://clang.llvm.org/doxygen/VTableBuilder_8cpp_source.html

#pragma once

#pragma region Configuration

#if BIFROST_DEBUG
#define BIFROST_ENABLE_DEBUG 1
#else
#define BIFROST_ENABLE_DEBUG 0
#endif

#ifndef BIFROST_CACHE_LINE
#define BIFROST_CACHE_LINE 64
#endif

#if BIFROST_NO_INCLUDE
#define BIFROST_ENABLE_INCLUDE 0
#else
#define BIFROST_ENABLE_INCLUDE 1
#endif

#pragma endregion

#include <cstdint>

#if BIFROST_ENABLE_INCLUDE
BIFROST_PLUGIN_INCLUDES
#endif

#define BIFROST_NAMESPACE_BEGIN namespace BIFROST_NAMESPACE {
#define BIFROST_NAMESPACE_END }

#pragma region Plugin Interface

struct bfp_PluginContext_t;

BIFROST_NAMESPACE_BEGIN

#ifdef _MSC_VER
#define BIFROST_CACHE_ALIGN __declspec(align(BIFROST_CACHE_LINE))
#else
#define BIFROST_CACHE_ALIGN
#endif

/// Plugin interface to be implemented
///
/// Register your plugin via the macro `BIFROST_REGISTER_PLUGIN`.
BIFROST_CACHE_ALIGN class Plugin {
 public:
  /// Access the singleton instance
  static Plugin& Get();

  /// Constructor
  Plugin();

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
  enum class Identifier : std::uint64_t { 
    __bifrost_first__ = 0, 
    BIFROST_PLUGIN_IDENTIFIER 
    NumIdentifiers 
  };

  /// Convert Identifier to string
  static const char* ToString(Identifier identifer);

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

  /// Called if a fatal exception occurred - by default logs to Error and throws std::runtime_error
  ///
  /// Note that this function may be called *before* `SetUp` has been invoked.
  ///
  /// @param[in] msg   Error message which has to be '\0' terminated
  virtual void FatalError(const char* msg) const;

  //
  // HOOKING
  //

  /// Hook description
  BIFROST_CACHE_ALIGN class Hook {
   public:
    friend class Plugin;

    /// Get the function pointer to the original function/method
    inline void* GetOriginal() const noexcept { return m_original; }

    /// Get the function pointer to the override (i.e detour) of the original function/method
    inline void* GetOverride() const noexcept { return m_override; }

    /// Get the associated identifier
    Identifier GetIdentifier() const noexcept { return m_identifier; }

   private:
    void* _GetTarget() const noexcept;
    void _SetTarget(void* target) noexcept;
    void _SetOverride(void* override) noexcept;
    void _SetOriginal(void* original) noexcept;
    void _SetIdentifier(Identifier identifier) noexcept;

   private:
    void* m_original = nullptr;
    void* m_target = nullptr;
    void* m_override = nullptr;
    Identifier m_identifier = Identifier::__bifrost_first__;
  };
  using AlignedHook = BIFROST_CACHE_ALIGN struct { Hook hook; };

  /// Hook the function given by `identifier` to call `override` instead
  ///
  /// If the underlying function is part of a V-Table, `SetVTableHook` should be used for the first method. Calls `FatalError` if an error occurred.
  ///
  /// @param[in] identifier Identifier of the function to override
  /// @param[in] override   Function pointer to use for the override (type has to match!)
  /// @param[in] priority   The higher the priority the earlier the function will placed in the hook chain (the highest priority function will be called first).
  /// @returns a reference to the hook object
  Hook* SetHook(Identifier identifier, void* override);
  Hook* SetHook(const char* identifier, void* override);
  Hook* SetHook(Identifier identifier, void* override, std::uint32_t priority);
  Hook* SetHook(const char* identifier, void* override, std::uint32_t priority);

  /// Get the default hook priority
  std::uint32_t GetDefaultHookPriority() const noexcept;

  /// Hook the method (`identifier`) in the VTable given by `instance` to call `override` instead
  ///
  /// The per object VTable is cached after the first call and `SetHook` can be used subsequently. Calls `FatalError` if an error occurred.
  ///
  /// @param[in] identifier Identifier of the method to override
  /// @param[in] identifier Instance of an object to extract the VTable from
  /// @param[in] override   Function pointer to use for the override (type has to match!)
  /// @returns a reference to the hook object
  Hook* SetVTableHook(Identifier identifier, void* instance, void* override);
  Hook* SetVTableHook(const char* identifier, void* instance, void* override);
  Hook* SetVTableHook(Identifier identifier, void* instance, void* override, std::uint32_t priority);
  Hook* SetVTableHook(const char* identifier, void* instance, void* override, std::uint32_t priority);

  /// Removes an already created the hook given by `identifier`
  ///
  /// If the hook has already been set, the hook is first deactivated and a new Hook object is created.
  /// Calls `FatalError` if an error occurred.
  ///
  /// @param[in] identifier Identifier of the function to override
  void RemoveHook(Identifier identifier);
  void RemoveHook(const char* identifier);
  void RemoveHook(Hook* hook);

  /// Remove the hook of each `identifier`
  template <class... IdentifierT>
  void RemoveHooks(IdentifierT&&... identifier) {
    RemoveHook(identifier)...;
  }

	/// Disables all created hooks
  void RemoveAllHooks() noexcept;

  /// Get an already created hook
  ///
  /// @param[in] identifier  Identifier of the hook
  /// @returns a reference to the hook object
  Hook* GetHook(Identifier identifier) noexcept;
  Hook* GetHook(const char* identifier);

  template <Identifier identifer>
  inline constexpr Hook* GetHook() noexcept {
    return &s_hooks[(std::uint64_t)identifer].hook;
  }

  /// Check if there was a hook created for `identifier`
  ///
  /// @param[in] identifier  Identifier of the hook
  /// @returns `true` if a hook has been set, false otherwise.
  bool HasHook(Identifier identifier) noexcept;
  bool HasHook(const char* identifier);

  //
  // HELPER
  //

  /// Access the arguments which were passed to the plugin
  const char* GetArguments() const noexcept;

  /// Severity level
  enum class LogLevel : unsigned int { Trace = 0, Debug, Info, Warn, Error, Disable };

  /// Log a message - calls Error on failure
  ///
  /// @param[in] level         Severity level
  /// @param[in] msg           Message which has to be '\0' terminated
  /// @param[in] ignoreErrors  Don't call `FatalError` if something goes wrong
  void Log(LogLevel level, const char* msg, bool ignoreErrors = false) const;

  //
  // INTERNAL
  //
  bfp_PluginContext_t* _GetPlugin();
  void _SetUpImpl(bfp_PluginContext_t*);
  void _TearDownImpl(bool);
  void _SetArguments(const char*);

 private:
  static BIFROST_CACHE_ALIGN Plugin* s_instance;
  static BIFROST_CACHE_ALIGN const char* s_name;
  static BIFROST_CACHE_ALIGN AlignedHook s_hooks[(std::uint64_t)Identifier::NumIdentifiers];

  class PluginImpl;
  PluginImpl* m_impl;
};

BIFROST_NAMESPACE_END

/// Register a plugin
///
/// (!) Do not place this macro inside a namespace
#define BIFROST_REGISTER_PLUGIN(plugin) BIFROST_REGISTER_PLUGIN_IMPL(plugin)

#define BIFROST_REGISTER_PLUGIN_IMPL(plugin)  \
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

// Implementation of bf_this
#define _bf_this _bf_concat(_bf_this_, _bf_concat(_bf_namespace, bf_id))

// Implementation of bf_args
#define _bf_args _bf_concat(_bf_args_, _bf_concat(_bf_namespace, bf_id))

/// bf_override
#define bf_override(name) _bf_func_decl_ret name(_bf_func_decl_args)

/// bf_original
#define bf_original(...) _bf_original(__VA_ARGS__)

/// bf_original
#define bf_this _bf_this

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

BIFROST_PLUGIN_DSL_DEF

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

#include <array>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <numeric>
#include <mutex>
#include <stdexcept>
#include <string>
#include <sstream>
#include <unordered_map>

#include "bifrost/api/plugin.h"

BIFROST_NAMESPACE_BEGIN

/// Available modules
enum class Module : std::uint32_t {
  __bifrost_first__ = 0,
  BIFROST_PLUGIN_MODULE 
	NumModule,
};

/// Hooking types
enum class HookType : std::uint32_t { __bifrost_first__ = std::numeric_limits<std::uint32_t>::max(), CFunction = BFP_CFUNCTION, VTable = BFP_VTABLE };

namespace {

/// Function used to determine the name of this DLL
static bool FunctionInThisDll() { return true; }

/// Format ``fmt`` using ``args``
template <class... Args>
inline void StringFormat(std::string& str, const char* fmt, Args&&... args) {
  if (str.empty()) str.resize(std::strlen(fmt) * 2);
  if (str.size() == 0) return;

  int size = 0;
  while ((size = std::snprintf(str.data(), str.size(), fmt, std::forward<Args>(args)...)) < 0 || size >= str.size()) {
    str.resize(str.size() * 2);
  }
  str = str.substr(0, size);
}

/// Format ``fmt`` using ``args``
template <class... Args>
inline std::string StringFormat(const char* fmt, Args&&... args) {
  std::string str;
  StringFormat(str, fmt, std::forward<Args>(args)...);
  return str;
}

/// Format ``fmt`` using ``args``
template <class... Args>
inline void WStringFormat(std::wstring& str, const wchar_t* fmt, Args&&... args) {
  if (str.empty()) str.resize(std::wcslen(fmt) * 2);
  if (str.size() == 0) return;

  int size = 0;
  while ((size = _snwprintf(str.data(), str.size(), fmt, std::forward<Args>(args)...)) < 0 || size >= str.size()) {
    str.resize(str.size() * 2);
  }
  str = str.substr(0, size);
}

/// Format ``fmt`` using ``args``
template <class... Args>
inline std::wstring StringFormat(const wchar_t* fmt, Args&&... args) {
  std::wstring str;
  WStringFormat(str, fmt, std::forward<Args>(args)...);
  return str;
}

/// Convert string to wstring
static std::wstring StringToWString(const std::string& s) {
  int len;
  int slength = (int)s.length() + 1;
  len = ::MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
  std::wstring r(len, L'\0');
  ::MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, &r[0], len);
  return r;
}

/// Convert wstring to string
static std::string WStringToString(const std::wstring& s) {
  int len;
  int slength = (int)s.length();
  len = ::WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
  std::string r(len, '\0');
  ::WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, &r[0], len, 0, 0);
  return r;
}

/// Get the most recent Win32 error
static std::string GetLastWin32Error() {
  DWORD errorCode = ::GetLastError();
  if (errorCode == 0) return "Unknown Error.";

  LPSTR messageBuffer = nullptr;
  size_t size = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode,
                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

  std::string message(messageBuffer, size - 1);
  ::LocalFree(messageBuffer);
  return message;
}

/// Get the current module
static HMODULE GetCurrentModule() {
  HMODULE hModule = nullptr;
  ::GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&FunctionInThisDll, &hModule);
  return hModule;
}

/// Get the filename of the module
static std::string GetModuleName(HMODULE hModule) {
  if (hModule != nullptr) {
    // Extract the full path
    std::string path(MAX_PATH, '\0');
    DWORD size = 0;
    do {
      path.resize(path.size() * 2);
      size = ::GetModuleFileNameA(hModule, (LPSTR)path.c_str(), (DWORD)path.size());
    } while (size == ERROR_INSUFFICIENT_BUFFER);
    return path.substr(0, size);
  }
  return {};
}

/// Issue `msg` to stderr and write to disk if `success` is false
static void Check(const char* msg, bool success, bool isWin32) {
  if (!success) {
    auto errMsg = std::string(msg);
    if (isWin32) {
      errMsg += ": " + GetLastWin32Error();
    }

    // Write to stderr
    std::cerr << "[ERROR] " << errMsg << std::endl;

    // Extract the filename
    auto path = GetModuleName(GetCurrentModule());
    if (path.size() > 0) {
      auto idx = path.find_last_of("\\/");
      if (idx != -1) path = path.substr(idx + 1, path.size() - idx - 1);
    } else {
      path = "plugin";
    }

    // Write to file
    std::ofstream ofs("log." + path + ".txt");
    ofs << errMsg << std::endl;

    throw std::runtime_error(errMsg.c_str());
  }
}

/// Interaction with bifrost_plugin.dll
class BifrostPluginApi {
 public:
#define BIFROST_PLUGIN_API_DECL(name) \
  using name##_fn = decltype(&name);  \
  name##_fn name;
#define BIFROST_PLUGIN_API_DEF(name) Check("GetProcAddress: " #name, (name = (name##_fn)::GetProcAddress(hModule, #name)) != NULL, true);

  BIFROST_PLUGIN_API_DECL(bfp_GetVersion)
  BIFROST_PLUGIN_API_DECL(bfp_PluginInit)
  BIFROST_PLUGIN_API_DECL(bfp_PluginFree)
  BIFROST_PLUGIN_API_DECL(bfp_StringFree)
  BIFROST_PLUGIN_API_DECL(bfp_PluginGetLastError)
  BIFROST_PLUGIN_API_DECL(bfp_PluginLog)
  BIFROST_PLUGIN_API_DECL(bfp_PluginSetUpStart)
  BIFROST_PLUGIN_API_DECL(bfp_PluginSetUpEnd)
  BIFROST_PLUGIN_API_DECL(bfp_PluginTearDownStart)
  BIFROST_PLUGIN_API_DECL(bfp_PluginTearDownEnd)
  BIFROST_PLUGIN_API_DECL(bfp_HookSet)
  BIFROST_PLUGIN_API_DECL(bfp_HookRemove)
  BIFROST_PLUGIN_API_DECL(bfp_HookEnableDebug)

  BifrostPluginApi() {
    HMODULE hModule = NULL;
    Check("LoadLibrary: bifrost_plugin.dll", (hModule = ::LoadLibraryW(L"bifrost_plugin.dll")) != NULL, true);
    BIFROST_PLUGIN_API_DEF(bfp_GetVersion)
    BIFROST_PLUGIN_API_DEF(bfp_PluginInit)
    BIFROST_PLUGIN_API_DEF(bfp_PluginFree)
    BIFROST_PLUGIN_API_DEF(bfp_StringFree)
    BIFROST_PLUGIN_API_DEF(bfp_PluginGetLastError)
    BIFROST_PLUGIN_API_DEF(bfp_PluginLog)
    BIFROST_PLUGIN_API_DEF(bfp_PluginSetUpStart)
    BIFROST_PLUGIN_API_DEF(bfp_PluginSetUpEnd)
    BIFROST_PLUGIN_API_DEF(bfp_PluginTearDownStart)
    BIFROST_PLUGIN_API_DEF(bfp_PluginTearDownEnd)
    BIFROST_PLUGIN_API_DEF(bfp_HookSet)
    BIFROST_PLUGIN_API_DEF(bfp_HookRemove)
    BIFROST_PLUGIN_API_DEF(bfp_HookEnableDebug)

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
      Check(ss.str().c_str(), false, false);
    }
  }

 private:
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

/// Convert a string to the Identifier enum
static Plugin::Identifier StringToIdentifier(const char* identifer) {
  static BIFROST_CACHE_ALIGN std::unordered_map<std::string, Plugin::Identifier> map{BIFROST_PLUGIN_STRING_TO_IDENTIFIER};

  auto it = map.find(identifer);
  if (it != map.end()) Plugin::Get().FatalError(StringFormat("Identifier \"%s\" does not exist", identifer).c_str());

  return it->second;
}

/// Convert an Identifier to string
static const char* IdentifierToString(Plugin::Identifier identifier) {
  static constexpr BIFROST_CACHE_ALIGN std::array<const char*, (std::uint64_t)Plugin::Identifier::NumIdentifiers + 1> map{
      "<invalid>",
      BIFROST_PLUGIN_IDENTIFIER_TO_STRING 
			"<invalid>",
  };
  return map[(std::uint64_t)identifier];
}

/// Get the original function name of an Identifier
static const char* IdentiferToFunctionName(Plugin::Identifier identifier) {
  static constexpr BIFROST_CACHE_ALIGN std::array<const char*, (std::uint64_t)Plugin::Identifier::NumIdentifiers + 1> map{
      "<invalid>",
      BIFROST_PLUGIN_IDENTIFIER_TO_FUNCTION_NAME 
			"<invalid>",
  };
  return map[(std::uint64_t)identifier];
}

/// Convert an Identifier to the associated module
static Module IdentifierToModule(Plugin::Identifier identifier) {
  static constexpr BIFROST_CACHE_ALIGN std::array<Module, (std::uint64_t)Plugin::Identifier::NumIdentifiers + 1> map{
      Module::__bifrost_first__,
      BIFROST_PLUGIN_IDENTIFIER_TO_MODULE 
			Module::__bifrost_first__,
  };
  return map[(std::uint64_t)identifier];
}

/// Convert an Identifier to the associated module
static HookType IdentifierToHookType(Plugin::Identifier identifier) {
  static constexpr BIFROST_CACHE_ALIGN std::array<HookType, (std::uint64_t)Plugin::Identifier::NumIdentifiers + 1> map{
      HookType::__bifrost_first__,
      BIFROST_PLUGIN_IDENTIFIER_TO_HOOK_TYPE 
			HookType::__bifrost_first__,
  };
  return map[(std::uint64_t)identifier];
}

/// Convert a Module enum to string
static const wchar_t* ModuleToString(Module module) {
  static constexpr BIFROST_CACHE_ALIGN std::array<const wchar_t*, (std::uint32_t)Module::NumModule + 1> map{
      L"<invalid>",
      BIFROST_PLUGIN_MODULE_TO_STRING 
			L"<invalid>",
  };
  return map[(std::uint32_t)module];
}

template <class T>
static void ForEachIdentifer(T&& func) {
  for (std::uint64_t i = (std::uint64_t)Plugin::Identifier::__bifrost_first__ + 1; i < (std::uint64_t)Plugin::Identifier::NumIdentifiers; ++i) {
    func((Plugin::Identifier)i);
  }
}

}  // namespace

BIFROST_CACHE_ALIGN Plugin* Plugin::s_instance = nullptr;

BIFROST_CACHE_ALIGN Plugin::AlignedHook Plugin::s_hooks[(std::uint64_t)Plugin::Identifier::NumIdentifiers];

const char* Plugin::ToString(Identifier identifer) { return IdentifierToString(identifer); }

BIFROST_CACHE_ALIGN class Plugin::PluginImpl {
 public:
  /// Map of modules to HMODULE
  BIFROST_CACHE_ALIGN HMODULE Modules[(std::uint64_t)Module::NumModule] = {nullptr};

  /// Context of the plugin (holds shared memory etc.)
  bfp_PluginContext* Context = nullptr;

  /// Was the plugin already initialized?
  bool Init = false;

  /// Arguments passed to the plugin
  std::string Arguments;
};

Plugin::Plugin() { m_impl = new PluginImpl; }

Plugin::~Plugin() {
  RemoveAllHooks();
  delete m_impl;
}

bfp_PluginContext_t* Plugin::_GetPlugin() { return m_impl->Context; }

void Plugin::Log(Plugin::LogLevel level, const char* msg, bool ignoreErrors) const {
  auto& api = GetApi();
  if (api.bfp_PluginLog(m_impl->Context, (uint32_t)level, GetName(), msg) != BFP_OK) {
    if (!ignoreErrors) {
      FatalError(api.bfp_PluginGetLastError(m_impl->Context));
    }
  }
}

const char* Plugin::GetArguments() const noexcept { return m_impl->Arguments.c_str(); }

void Plugin::FatalError(const char* msg) const {
  Log(LogLevel::Error, msg, true);
  throw std::runtime_error(msg);
}

void* Plugin::Hook::_GetTarget() const noexcept { return m_target; }

void Plugin::Hook::_SetTarget(void* target) noexcept { m_target = target; }

void Plugin::Hook::_SetOverride(void* override) noexcept { m_override = override; }

void Plugin::Hook::_SetOriginal(void* original) noexcept { m_original = original; }

void Plugin::Hook::_SetIdentifier(Identifier identifer) noexcept { m_identifier = identifer; }

Plugin::Hook* Plugin::SetHook(Plugin::Identifier identifier, void* override) { return SetHook(identifier, override, GetDefaultHookPriority()); }

Plugin::Hook* Plugin::SetHook(const char* identifier, void* override) { return SetHook(identifier, override, GetDefaultHookPriority()); }

Plugin::Hook* Plugin::SetHook(const char* identifier, void* override, std::uint32_t priority) {
  return SetHook(StringToIdentifier(identifier), override, priority);
}

Plugin::Hook* Plugin::SetHook(Plugin::Identifier identifier, void* override, std::uint32_t priority) {
  auto& api = GetApi();
  Hook* hook = GetHook(identifier);

  // Do we have a target?
  if (hook->_GetTarget() == nullptr) {
    FatalError(StringFormat("Failed to set hook for %s: function was not loaded successfully", IdentiferToFunctionName(identifier)).c_str());
  }

  // Set the hook
  void* newOriginal = hook->GetOriginal();
  void* newOverride = override;

  bfp_HookSetDesc desc = {};
  desc.Type = static_cast<bfp_HookType>(IdentifierToHookType(hook->GetIdentifier()));
  desc.Priority = priority;
  desc.Target = hook->_GetTarget();
  desc.Detour = newOverride;

  if (api.bfp_HookSet(m_impl->Context, &desc, &newOriginal) != BFP_OK) {
    FatalError(api.bfp_PluginGetLastError(m_impl->Context));
  }

  hook->_SetOriginal(newOriginal);
  hook->_SetOverride(newOverride);
  return hook;
}

//void Plugin::EnableHooks(const char** identifers, std::uint32_t num) {
//  Identifier* identiferEnums = (Identifier*)::_alloca(sizeof(Identifier) * num);
//  for (std::uint32_t i = 0; i < num; ++i) identiferEnums[i] = StringToIdentifier(identifers[i]);
//  EnableHooks(identiferEnums, num);
//}

void Plugin::RemoveAllHooks() noexcept {
  ForEachIdentifer([this](Identifier identifier) { RemoveHook(identifier); });
}

void Plugin::RemoveHook(Identifier identifier) { RemoveHook(GetHook(identifier)); }

void Plugin::RemoveHook(const char* identifier) { RemoveHook(StringToIdentifier(identifier)); }

void Plugin::RemoveHook(Hook* hook) {
  auto& api = GetApi();

  // Function was not loaded successfully
  if (hook->_GetTarget() == nullptr) return;

  // No hook has been registered
  if (hook->GetOverride() == nullptr) return;

  bfp_HookRemoveDesc desc = {};
  desc.Type = static_cast<bfp_HookType>(IdentifierToHookType(hook->GetIdentifier()));
  desc.Target = hook->_GetTarget();

  if (api.bfp_HookRemove(m_impl->Context, &desc) != BFP_OK) {
    FatalError(api.bfp_PluginGetLastError(m_impl->Context));
  }

  hook->_SetOriginal(hook->_GetTarget());
  hook->_SetOverride(nullptr);
}

std::uint32_t Plugin::GetDefaultHookPriority() const noexcept { return BIFROST_PLUGIN_DEFAULT_HookSetDesc_Priority; }

Plugin::Hook* Plugin::GetHook(Plugin::Identifier identifier) noexcept { return &s_hooks[(std::uint64_t)identifier].hook; }

Plugin::Hook* Plugin::GetHook(const char* identifier) { return GetHook(StringToIdentifier(identifier)); }

bool Plugin::HasHook(Plugin::Identifier identifier) noexcept { return GetHook(identifier) != nullptr; }

bool Plugin::HasHook(const char* identifier) { return HasHook(StringToIdentifier(identifier)); }

const char* Plugin::GetName() const { return s_name; }

void Plugin::_SetUpImpl(bfp_PluginContext_t* ctx) {
  auto& api = GetApi();
  m_impl->Context = ctx;

  if (m_impl->Init) throw std::runtime_error("Plugin already set up");

#if BIFROST_ENABLE_DEBUG
  // Enable debug layer for symbols
  if (api.bfp_HookEnableDebug(ctx) != BFP_OK) {
    Log(LogLevel::Warn, api.bfp_PluginGetLastError(m_impl->Context));
  }
#endif

  // Set the identifier
  for (std::uint64_t identiferIndex = (std::uint64_t)Identifier::__bifrost_first__ + 1; identiferIndex < (std::uint64_t)Identifier::NumIdentifiers;
       ++identiferIndex) {
    Hook& hook = s_hooks[identiferIndex].hook;
    hook._SetIdentifier((Identifier)identiferIndex);
  }

  // Load all externally referenced libraries
  for (std::uint32_t moduleIndex = (std::uint32_t)Identifier::__bifrost_first__ + 1; moduleIndex < (std::uint32_t)Module::NumModule; ++moduleIndex) {
    auto moduleString = ModuleToString((Module)moduleIndex);

#ifdef BIFROST_ENABLE_DEBUG
    Log(LogLevel::Debug, StringFormat("Loading library \"%s\"", WStringToString(moduleString).c_str()).c_str());
#endif

    HMODULE hModule = ::LoadLibraryW(moduleString);
    if (hModule == nullptr) {
      Log(LogLevel::Warn, StringFormat("Failed to load library \"%s\": %s", WStringToString(moduleString).c_str(), GetLastWin32Error().c_str()).c_str());
    }
    m_impl->Modules[moduleIndex] = hModule;
  }

  // Load all referenced C functions
  for (std::uint64_t identiferIndex = (std::uint64_t)Identifier::__bifrost_first__ + 1; identiferIndex < (std::uint64_t)Identifier::NumIdentifiers;
       ++identiferIndex) {
    Identifier identifer = (Identifier)identiferIndex;
    if (IdentifierToHookType(identifer) == HookType::CFunction) {
      const char* functionName = IdentiferToFunctionName(identifer);

#ifdef BIFROST_ENABLE_DEBUG
      Log(LogLevel::Trace, StringFormat("Loading function %s", functionName).c_str());
#endif

      // Check if the associated module has been loaded
      Module module = IdentifierToModule(identifer);
      HMODULE hModule = m_impl->Modules[(std::uint32_t)module];
      if (hModule == nullptr) {
        Log(LogLevel::Warn, StringFormat("Skipping loading of function %s: associated library \"%s\" was not successfully loaded", functionName,
                                         WStringToString(ModuleToString(module)).c_str())
                                .c_str());
        continue;
      }

      // Load the target function
      Hook& hook = s_hooks[identiferIndex].hook;
      hook._SetTarget(::GetProcAddress(hModule, functionName));
      hook._SetOriginal(hook._GetTarget());
      if (hook._GetTarget() == nullptr) {
        Log(LogLevel::Warn, StringFormat("Failed to load function %s: %s", functionName, GetLastWin32Error().c_str()).c_str());
      }
    }
  }

  // Call user provided set up
  SetUp();
  m_impl->Init = true;
}

void Plugin::_TearDownImpl(bool noFail) {
  if (noFail && !m_impl->Init) return;
  if (!m_impl->Init) throw std::runtime_error("Plugin not set up");
  TearDown();
  RemoveAllHooks();
  m_impl->Init = false;
}

void Plugin::_SetArguments(const char* arguments) { m_impl->Arguments = arguments; }

BIFROST_NAMESPACE_END

#include "bifrost/template/plugin_fwd.h"

BIFROST_PLUGIN_SETUP_PROC_DECL

BIFROST_PLUGIN_SETUP_PROC_DEF {
  using namespace BIFROST_NAMESPACE;
  auto& api = GetApi();

  Plugin* plugin = &Plugin::Get();
  const char* errMsg = nullptr;

  // Initialize the plugin & the internal hooking mechanism
  bfp_PluginContext* ctx = api.bfp_PluginInit(&errMsg);
  if (!ctx || errMsg != nullptr) {
    std::string errMsgStr = errMsg ? errMsg : "Unknown error in " BIFROST_PLUGIN_SETUP_PROC_NAME_STRING;
    api.bfp_StringFree(errMsg);
    Check(errMsgStr.c_str(), false, false);
  }

  // Setup the plugin (parse the arguments and connect to shared memory/set up logging)
  bfp_PluginSetUpArguments* args;
  if (api.bfp_PluginSetUpStart(ctx, plugin->GetName(), param, &args) != BFP_OK) plugin->FatalError(api.bfp_PluginGetLastError(ctx));

  plugin->_SetArguments(args->Arguments);
  plugin->_SetUpImpl((bfp_PluginContext_t*)ctx);

  if (api.bfp_PluginSetUpEnd(plugin->_GetPlugin(), plugin->GetName(), param, args) != BFP_OK) plugin->FatalError(api.bfp_PluginGetLastError(ctx));
  return 0;
}

BIFROST_PLUGIN_TEARDOWN_PROC_DECL

BIFROST_PLUGIN_TEARDOWN_PROC_DEF {
  using namespace BIFROST_NAMESPACE;
  auto& api = GetApi();

  Plugin* plugin = &Plugin::Get();
  bfp_PluginContext* ctx = (bfp_PluginContext*)plugin->_GetPlugin();

  bfp_PluginTearDownArguments* args;
  if (api.bfp_PluginTearDownStart(ctx, param, &args) != BFP_OK) plugin->FatalError(api.bfp_PluginGetLastError(ctx));

  plugin->_TearDownImpl(args->NoFail);

  if (api.bfp_PluginTearDownEnd(ctx, param, args) != BFP_OK) plugin->FatalError(api.bfp_PluginGetLastError(ctx));

  const char* errMsg = nullptr;
  api.bfp_PluginFree(ctx, &errMsg);
  if (errMsg != nullptr) {
    std::string errMsgStr = errMsg;
    api.bfp_StringFree(errMsg);
    Check(errMsgStr.c_str(), false, false);
  }
  return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) { return TRUE; }

#endif

#pragma endregion
