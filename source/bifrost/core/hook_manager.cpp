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
#pragma comment(lib, "dbghelp")
#pragma warning(pop)

namespace bifrost {

namespace {

/// Settings to configure the hooking mechanisms via file and env variables
class HookSettings {
 public:
  /// Debug mode enabled?
  bool Debug = false;

  /// Run DbgHelp in verbose mode?
  bool VerboseDbgHelp = false;

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
    VerboseDbgHelp = TryParseAsBool(ctx, "VerboseDbgHelp", j, "BIFROST_HOOK_VERBOSE_DBGHELP", VerboseDbgHelp);
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

    ctx->Logger().DebugFormat("  %-20s: %s", name, logExtract(value));
    return value;
  }
};

/// Wrapper for SYMBOL_INFO_PACKAGE structure
struct SymbolInfoPackage : public ::SYMBOL_INFO_PACKAGE {
  SymbolInfoPackage() {
    si.SizeOfStruct = sizeof(::SYMBOL_INFO);
    si.MaxNameLen = sizeof(name);
  }
};

/// Wrapper for IMAGEHLP_LINE64 structure
struct ImageHlpLine64 : public IMAGEHLP_LINE64 {
  ImageHlpLine64() { SizeOfStruct = sizeof(IMAGEHLP_LINE64); }
};

inline const char* GetConstCharPtr(const char* str) { return str; }
inline const char* GetConstCharPtr(const std::string& str) { return str.c_str(); }

#define BIFROST_CHECK_MH(call, reason)                                                         \
  if (MH_STATUS status; (status = call) != MH_OK) {                                            \
    throw Exception("Failed to %s: %s", GetConstCharPtr(reason), ::MH_StatusToString(status)); \
  }

#define BIFROST_LOG_DEBUG(...)              \
  if (m_debugMode) {                        \
    ctx->Logger().DebugFormat(__VA_ARGS__); \
  }

}  // namespace

class HookManager::Impl {
 public:
  void SetUp(Context* ctx) {
    BIFROST_LOCK_GUARD(m_mutex);

    // Allocate the settings
    m_settings = std::make_unique<HookSettings>(ctx);

    // Initialize MinHook
    BIFROST_CHECK_MH(MH_Initialize(), "initialize MinHook");

    // Initialize symbol loading
    if (m_settings->Debug) EnableDebugImpl(ctx);

    m_id = 0;
  }

  void TearDown(Context* ctx) {
    BIFROST_LOCK_GUARD(m_mutex);

    // Free MinHook
    BIFROST_CHECK_MH(MH_Uninitialize(), "uninitialize MinHook");

    // Cleanup symbol loading
    if (m_debugMode) {
      BIFROST_CHECK_WIN_CALL_CTX(ctx, ::SymCleanup(::GetCurrentProcess()));
      m_debugMode = false;
      m_symbolCache.clear();
    }
  }

  void HookCreate(u32 id, Context* ctx, void* target, void* detour, void** original) {
    BIFROST_LOCK_GUARD(m_mutex);
    BIFROST_LOG_DEBUG("Creating hook from %s to %s", SymbolFromAdress(ctx, target), SymbolFromAdress(ctx, detour));
    HookCreateImpl(id, ctx, target, detour, original);
  }

  void HookRemove(u32 id, Context* ctx, void* target) {
    BIFROST_LOCK_GUARD(m_mutex);
    BIFROST_LOG_DEBUG("Removing hook %s", SymbolFromAdress(ctx, target));
    HookRemoveImpl(id, ctx, target);
  }

  void HookEnable(u32 id, Context* ctx, void** targets, u32 num) {
    BIFROST_LOCK_GUARD(m_mutex);
    for (u32 i = 0; i < num; ++i) BIFROST_LOG_DEBUG("Enabling hook %s", SymbolFromAdress(ctx, targets[i]));
    HookEnableImpl(id, ctx, targets, num);
  }

  void HookDisable(u32 id, Context* ctx, void** targets, u32 num) {
    BIFROST_LOCK_GUARD(m_mutex);
    for (u32 i = 0; i < num; ++i) BIFROST_LOG_DEBUG("Disabling hook %s", SymbolFromAdress(ctx, targets[i]));
    HookDisableImpl(id, ctx, targets, num);
  }

  void EnableDebug(Context* ctx) {
    BIFROST_LOCK_GUARD(m_mutex);
    EnableDebugImpl(ctx);
  }

  u32 GetId() {
    BIFROST_LOCK_GUARD(m_mutex);
    return m_id++;
  }

 private:
  //
  // FUNCTION HOOKS
  //

  /// Per id hook description
  struct HookChainNode {
    u32 Id;
    bool Enabled;
    void* Override;
  };

  /// Per function hook description
  class HookDesc {
   public:
    friend class HookManager::Impl;

    HookDesc(void* original) : m_original(original) { m_hookChain.reserve(8); }

    /// Get the original function
    void* Original() { return m_original; }

    /// Get the node of id in the chain or NULL if no node exists for this id
    HookChainNode* GetHookChainNode(u32 id) {
      for (auto& node : m_hookChain) {
        if (node.Id == id) return &node;
      }
      return nullptr;
    }

    /// Add a new node
    void AddHookChainNode(HookChainNode&& node) { m_hookChain.emplace_back(std::move(node)); }

    /// Remove the node and potentially re-hook adjacent node(s)
    void RemoveHookChainNode(HookManager::Impl* impl, const HookChainNode* node) {
      if (&m_hookChain.back() == node) {
        // This is the last node in the chain, we can safely remove it
				
				// TODO:
				// 1) Last -> kill
				// 2) Freeze threads -> disable & remove current hook -> recreate hook from left to this one -> enable it? -> unfreeze threads
				// RAII to unfreeze
      }
    }

    /// Get the number of registered hooks
    u32 NumHooks() const { return m_hookChain.size(); }

   private:
    void* m_original;
    std::vector<HookChainNode> m_hookChain;
  };

  HookDesc* GetHookDesc(void* target) {
    auto it = m_hookTargetToDesc.find((u64)target);
    return it != m_hookTargetToDesc.end() ? &it->second : nullptr;
  }

  HookDesc* SetHookDesc(void* target, HookDesc desc) { return &m_hookTargetToDesc.emplace((u64)target, std::move(desc)).first->second; }

  void RemoveHookDesc(void* target) { m_hookTargetToDesc.erase((u64)target); }

  void MinHookCreateHook(Context* ctx, void* target, void* detour, void** original) {
    BIFROST_CHECK_MH(MH_CreateHook(target, detour, original),
                     StringFormat("to create hook from %s to %s", SymbolFromAdress(ctx, target), SymbolFromAdress(ctx, detour)));
  }

  void MinHookRemoveHook(Context* ctx, void* target) {
    BIFROST_CHECK_MH(MH_RemoveHook(target), StringFormat("to remove hook from %s", SymbolFromAdress(ctx, target)));
  }

  void MinHookEnableHook(Context* ctx, void* target) {
    BIFROST_CHECK_MH(MH_EnableHook(target), StringFormat("to enable hook from %s", SymbolFromAdress(ctx, target)));
  }

  void MinHookDisableHook(Context* ctx, void* target) {
    BIFROST_CHECK_MH(MH_DisableHook(target), StringFormat("to disable hook from %s", SymbolFromAdress(ctx, target)));
  }

  void HookCreateImpl(u32 id, Context* ctx, void* target, void* detour, void** original) {
    HookDesc* desc = GetHookDesc(target);

    if (!desc) {
      // No other hook exists, create it!
      MinHookCreateHook(ctx, target, detour, original);
      desc = SetHookDesc(target, HookDesc{original});
      desc->AddHookChainNode(HookChainNode{id, false, detour});
    } else {
      // There is already an existing hook on this target!
      HookChainNode* node = desc->GetHookChainNode(id);
      BIFROST_ASSERT(desc->NumHooks() != 0);

      if (node) {
        if (desc->NumHooks() == 1) {
          // We are the only hook that exists, recreate it
          HookRemoveImpl(id, ctx, target);
          HookCreateImpl(id, ctx, target, detour, original);
        } else {
          // There are multiple hooks registered on this target, we may have to re-hook some of them
          desc->RemoveHookChainNode(this, node);
        }
      } else {
        // This is the first hook for us, chain it to the already existing hook(s)
        desc->AddHookChainNode(HookChainNode{id, false, detour});
      }
    }
  }

  void HookRemoveImpl(u32 id, Context* ctx, void* target) {
    HookDesc* desc = GetHookDesc(target);
    if (!desc) return;

    HookChainNode* node = desc->GetHookChainNode(id);
    if (node) {
      // A hook as been registered for this target, remove it
      desc->RemoveHookChainNode(this, node);

      if (desc->NumHooks() == 0) {
        // This was the last hook in the chain of of this target, remove the entire hook
        MinHookRemoveHook(ctx, target);
        RemoveHookDesc(target);
      }
    }
  }

  void HookEnableImpl(u32 id, Context* ctx, void** targets, u32 num) {
    if (num == 0) return;

    if (num == 1) {
      if (targets[0]) {
        BIFROST_CHECK_MH(MH_EnableHook(targets[0]), StringFormat("enable hook for %s", SymbolFromAdress(ctx, targets[0])));
      }
    } else {
      u32 numQueued = 0;

      for (u32 i = 0; i < num; ++i) {
        if (targets[i]) {
          BIFROST_CHECK_MH(MH_QueueEnableHook(targets[i]), StringFormat("queue enable hook for %s", SymbolFromAdress(ctx, targets[i])));
          numQueued++;
        }
      }

      if (numQueued > 0) {
        BIFROST_CHECK_MH(MH_ApplyQueued(), "failed to apply queued opterations");
      }
    }
  }

  void HookDisableImpl(u32 id, Context* ctx, void** targets, u32 num) {
    if (num == 0) return;

    if (num == 1) {
      if (targets[0]) {
        BIFROST_CHECK_MH(MH_DisableHook(targets[0]), StringFormat("disable hook for %s", SymbolFromAdress(ctx, targets[0])));
      }
    } else {
      u32 numQueued = 0;
      for (u32 i = 0; i < num; ++i) {
        if (targets[i]) {
          BIFROST_CHECK_MH(MH_QueueDisableHook(targets[i]), StringFormat("queue disable hook for %s", SymbolFromAdress(ctx, targets[i])));
          numQueued++;
        }
      }

      if (numQueued > 0) {
        BIFROST_CHECK_MH(MH_ApplyQueued(), "failed to apply queued opterations");
      }
    }
  }

  //
  // SYMBOL DEBUG
  //

  /// Get the name of the symbol pointed to by `addr`
  const char* SymbolFromAdress(Context* ctx, void* addr) { return SymbolFromAdress(ctx, (u64)addr); }
  const char* SymbolFromAdress(Context* ctx, u64 addr) {
    // Is this symbol known?
    auto it = m_symbolCache.find(addr);
    if (it != m_symbolCache.end()) return it->second.c_str();

    if (m_debugMode) {
      DWORD64 dwDisplacement = 0;
      DWORD64 dwAddress = addr;
      SymbolInfoPackage info;

      // Load the symbol from the given address
      std::string symbolName;
      if (::SymFromAddr(::GetCurrentProcess(), dwAddress, &dwDisplacement, &info.si) == TRUE) {
        symbolName = std::string{info.si.Name, info.si.NameLen};

      } else {
        // Failed to load the symbol -> take address
        ctx->Logger().WarnFormat("Failed to get symbol name of 0x%08x: %s", addr, GetLastWin32Error().c_str());
        symbolName = StringFormat("0x%08x", addr);
      }
      return m_symbolCache.emplace(addr, std::move(symbolName)).first->second.c_str();
    } else {
      return m_symbolCache.emplace(addr, StringFormat("0x%08x", addr)).first->second.c_str();
    }
  }

  void EnableDebugImpl(Context* ctx) {
    if (!m_debugMode) {
      DWORD options = ::SymGetOptions();
      if (m_settings->VerboseDbgHelp) options |= SYMOPT_DEBUG;
      options |= SYMOPT_UNDNAME;
      ::SymSetOptions(options);

      ctx->Logger().Info("Loading symbols ...");
      BIFROST_CHECK_WIN_CALL_CTX(ctx, ::SymInitialize(::GetCurrentProcess(), NULL, TRUE) == TRUE);
      m_symbolCache.reserve(512);
      m_debugMode = true;
    }
  }

 private:
  SpinMutex m_mutex;
  std::unique_ptr<HookSettings> m_settings;

  // Function hooks
  std::unordered_map<u64, HookDesc> m_hookTargetToDesc;

  // Unique ids
  u32 m_id = 0;

  // Debug
  std::unordered_map<u64, std::string> m_symbolCache;
  bool m_debugMode = false;
};

HookManager::HookManager() { m_impl = std::make_unique<HookManager::Impl>(); }

HookManager::~HookManager() {}

void HookManager::SetUp(Context* ctx) { m_impl->SetUp(ctx); }

void HookManager::TearDown(Context* ctx) { m_impl->TearDown(ctx); }

u32 HookManager::GetId() { return m_impl->GetId(); }

void HookManager::HookCreate(u32 id, Context* ctx, void* target, void* detour, void** original) { m_impl->HookCreate(id, ctx, target, detour, original); }

void HookManager::HookRemove(u32 id, Context* ctx, void* target) { m_impl->HookRemove(id, ctx, target); }

void HookManager::HookEnable(u32 id, Context* ctx, void** targets, u32 num) { m_impl->HookEnable(id, ctx, targets, num); }

void HookManager::HookDisable(u32 id, Context* ctx, void** targets, u32 num) { m_impl->HookDisable(id, ctx, targets, num); }

void HookManager::EnableDebug(Context* ctx) { m_impl->EnableDebug(ctx); }

}  // namespace bifrost
