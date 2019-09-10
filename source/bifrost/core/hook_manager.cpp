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

#include "bifrost/core/hook_manager.h"
#include "bifrost/core/mutex.h"
#include "bifrost/core/exception.h"
#include "bifrost/core/error.h"
#include "bifrost/core/json.h"

#include "MinHook.h"

#pragma warning(push)
#pragma warning(disable : 4091)
#include <DbgHelp.h>
#pragma warning(pop)

namespace bifrost {

namespace {

/// Settings to configure the hooking mechanisms via file and env variables
class HookSettings {
 public:
  /// Debug mode enabled?
  bool Debug = true;

  HookSettings(Context* ctx) {
    // Try to read from hooks.json
    std::string hookFile = (std::filesystem::current_path() / L"hook.json").string();
    if (const char* var; var = std::getenv("BIFROST_HOOK_FILE")) hookFile = var;

    Json j;
    if (std::filesystem::exists(hookFile)) {
      std::ifstream ifs(hookFile);
      try {
        j = Json::parse(ifs);
      } catch (std::exception& e) {
        ctx->Logger().WarnFormat("Failed to read hook settings file \"%s\": %s", hookFile.c_str(), e.what());
      }
    }

    ctx->Logger().Debug("Hook settings:");
    Debug = TryParseAsBool(ctx, "Debug", j, "BIFROST_HOOK_DEBUG", Debug);
  }

 private:
  bool TryParseAsBool(Context* ctx, const char* name, const Json& j, const char* envVar, bool defaultValue) {
    return TryParseImpl(
        ctx, name, j, [](const Json& value) { return value.get<bool>(); }, envVar,
        [&defaultValue](const char* value) -> bool {
          std::string_view view = value;
          if (view == "1" || StringCompareCaseInsensitive(view, "true")) return true;
          if (view == "0" || StringCompareCaseInsensitive(view, "false")) return false;
          throw Exception("Failed to parse string \"%s\" as boolean", value);
          return defaultValue;
        },
        [](bool value) { return value ? "true" : "false"; }, defaultValue);
  }

  bool TryParseAsInt(Context* ctx, const char* name, const Json& j, const char* envVar, i32 defaultValue) {
    return TryParseImpl(
        ctx, name, j, [](const Json& value) { return value.get<i32>(); }, envVar,
        [&defaultValue](const char* value) {
          i32 v = defaultValue;
          std::istringstream ss(value);
          ss >> v;
          if (ss.fail()) throw Exception("Failed to parse string \"%s\" as integer", value);
          return v;
        },
        [](i32 value) { return std::to_string(value); }, defaultValue);
  }

  template <class T, class JsonExtractFuncT, class EnvExtractFuncT, class LogExtractT>
  T TryParseImpl(Context* ctx, const char* name, const Json& j, JsonExtractFuncT&& jsonExtractFunc, const char* envVar, EnvExtractFuncT&& envExtractFunc,
                 LogExtractT&& logExtract, T defaultValue) {
    T value = defaultValue;

    // Try to extract the value from json
    if (j.count(name)) {
      try {
        value = jsonExtractFunc(j[name]);
      } catch (std::exception& e) {
        ctx->Logger().WarnFormat("Failed to extract hook setting \"%s\" from Json: %s", name, e.what());
      }
    }

    // Try to extract the value from env variable
    if (const char* env; env = std::getenv(envVar)) {
      try {
        value = envExtractFunc(env);
      } catch (std::exception& e) {
        ctx->Logger().WarnFormat("Failed to extract hook setting \"%s\" from env variable %s: %s", name, envVar, e.what());
      }
    }

    ctx->Logger().DebugFormat("  %-10s: %s", name, logExtract(value));
    return value;
  }
};

inline const char* GetConstCharPtr(const char* str) { return str; }
inline const char* GetConstCharPtr(const std::string& str) { return str.c_str(); }

#define BIFROST_CHECK_MH(ret, reason)                                                       \
  if (ret != MH_OK) {                                                                       \
    throw Exception("Failed to %s: %s", GetConstCharPtr(reason), ::MH_StatusToString(ret)); \
  }

#define BIFROST_LOG_DEBUG(...)              \
  if (m_settings->Debug) {                  \
    ctx->Logger().DebugFormat(__VA_ARGS__); \
  }

}  // namespace

class HookManager::Impl {
 public:
  void SetUp(Context* ctx) {
    // Allocate the settings
    m_settings = std::make_unique<HookSettings>(ctx);

    // Initialize MinHook
    BIFROST_CHECK_MH(MH_Initialize(), "initialize MinHook");

    // Initialize symbol loading
    if (m_settings->Debug) {
      BIFROST_CHECK_WIN_CALL_CTX(ctx, SymInitialize(GetCurrentProcess(), NULL, FALSE));
    }

    m_id = 0;
  }

  void TearDown(Context* ctx) {
    // Free MinHook
    BIFROST_CHECK_MH(MH_Uninitialize(), "uninitialize MinHook");

    // Cleanup symbol loading
    if (m_settings->Debug) {
      BIFROST_CHECK_WIN_CALL_CTX(ctx, SymCleanup(GetCurrentProcess()));
    }
  }

  //
  // FUNCTION HOOKS
  //

  /// Per id hook description
  class HookDescPerId {
   public:
    HookDescPerId(u32 id) : m_id(id) {}

    /// Get the associated id
    u32 Id() { return m_id; }

   private:
    u32 m_id;
  };

  /// Per function hook description
  class HookDesc {
   public:
    HookDesc(u32 id, void* original) : m_original(original) { Add(id); }

    /// Get the original function
    void* Original() { return m_original; }

    /// Get the description of the id if available
    HookDescPerId* PerIdDesc(u32 id) {
      auto it = m_idToIt.find(id);
      return it != m_idToIt.end() ? &(*it->second) : nullptr;
    }

    std::size_t NumHooks() { return m_idToIt.size(); }

   private:
    void Add(u32 id) {
      m_perIdDesc.emplace_back(id);
      m_idToIt[id] = --m_perIdDesc.end();
    }

   private:
    void* m_original;

    std::unordered_map<u32, std::list<HookDescPerId>::iterator> m_idToIt;
    std::list<HookDescPerId> m_perIdDesc;
  };

  void HookCreate(u32 id, Context* ctx, void* target, void* detour, bool enable, void** original) {
    BIFROST_LOCK_GUARD(m_mutex);
    BIFROST_LOG_DEBUG("Creating %s hook from \"%s\" to \"%s\"", enable ? "and enabling" : "", SymbolFromAdress(ctx, target), SymbolFromAdress(ctx, detour));
    HookCreateImpl(id, ctx, target, detour, enable, original);
  }

  void HookCreateImpl(u32 id, Context* ctx, void* target, void* detour, bool enable, void** original) {
    auto desc = GetHookDesc(target);
    if (!desc) {
      // No other hook exists, create it!
      BIFROST_CHECK_MH(MH_CreateHook(target, detour, original),
                       StringFormat("creating hook from \"%s\" to \"%s\"", SymbolFromAdress(ctx, target), SymbolFromAdress(ctx, target)));
      desc = SetHookDesc(target, {id, original});
    }

    if (desc->NumHooks() > 1) {
    }
    if (enable) HookEnableImpl(id, ctx, target);
  }

  void HookEnable(u32 id, Context* ctx, void* target) {
    BIFROST_LOCK_GUARD(m_mutex);
    BIFROST_LOG_DEBUG("Enabling hook %s", SymbolFromAdress(ctx, target));
    HookEnableImpl(id, ctx, target);
  }

  void HookEnableImpl(u32 id, Context* ctx, void* target) {
    BIFROST_CHECK_MH(MH_EnableHook(target), StringFormat("enable hook for \"%s\"", SymbolFromAdress(ctx, target)));
  }

  //
  // UNIQUE IDS
  //

  /// Get a unique identifier
  u32 GetId() {
    BIFROST_LOCK_GUARD(m_mutex);
    return m_id++;
  }

  //
  // DEBUG
  //

  /// Get the name of the symbol pointed to by `addr`
  std::string SymbolFromAdress(Context* ctx, void* addr) { return SymbolFromAdress(ctx, (u64)addr); }
  std::string SymbolFromAdress(Context* ctx, u64 addr) {
    if (m_settings->Debug) return StringFormat("0x%08x", addr);

    // Is this symbol known?
    auto it = m_symbolCache.find(addr);
    if (it != m_symbolCache.end()) return it->second;

    // Try to get the name of it
    DWORD64 dwDisplacement = 0;
    DWORD64 dwAddress = addr;

    char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
    ::PSYMBOL_INFO pSymbol = (::PSYMBOL_INFO)buffer;
    pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    pSymbol->MaxNameLen = MAX_SYM_NAME;

    BIFROST_CHECK_WIN_CALL_CTX(ctx, SymFromAddr(GetCurrentProcess(), dwAddress, &dwDisplacement, pSymbol) == TRUE);
    return m_symbolCache.emplace(addr, std::string{pSymbol->Name, pSymbol->NameLen}).first->second;
  }

  //
  // UTILITY
  //

  inline HookDesc* GetHookDesc(void* target) {
    auto it = m_hookTargetToDesc.find((u64)target);
    return it != m_hookTargetToDesc.end() ? &it->second : nullptr;
  }

  inline HookDesc* SetHookDesc(void* target, HookDesc desc) { return &m_hookTargetToDesc.emplace((u64)target, std::move(desc)).first->second; }

 private:
  SpinMutex m_mutex;
  std::unique_ptr<HookSettings> m_settings;

  // Function hooks
  std::unordered_map<u64, HookDesc> m_hookTargetToDesc;

  // Unique ids
  u32 m_id = 0;

  // Debug
  std::unordered_map<u64, std::string> m_symbolCache;
};

HookManager::HookManager() { m_impl = std::make_unique<HookManager::Impl>(); }

HookManager::~HookManager() {}

void HookManager::SetUp(Context* ctx) { m_impl->SetUp(ctx); }

void HookManager::TearDown(Context* ctx) { m_impl->TearDown(ctx); }

u32 HookManager::GetId() { return m_impl->GetId(); }

void HookManager::HookCreate(u32 id, Context* ctx, void* target, void* detour, bool enable, void** original) {
  m_impl->HookCreate(id, ctx, target, detour, enable, original);
}

void HookManager::HookRemove(u32 id, Context* ctx, void* target) {}

void HookManager::HookEnable(u32 id, Context* ctx, void* target) { m_impl->HookEnable(id, ctx, target); }

void HookManager::HookDisable(u32 id, Context* ctx, void* target) {}

}  // namespace bifrost