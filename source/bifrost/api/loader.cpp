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
#include "bifrost/api/plugin_context.h"
#include "bifrost/core/buffered_logger.h"
#include "bifrost/core/context.h"
#include "bifrost/core/error.h"
#include "bifrost/core/exception.h"
#include "bifrost/core/injector_param.h"
#include "bifrost/core/module_loader.h"
#include "bifrost/core/plugin_param.h"
#include "bifrost/core/shared_memory.h"
#include "bifrost/core/sm_log_stash.h"

#include "bifrost/template/plugin_fwd.h"

using namespace bifrost;
using namespace bifrost::api;

extern "C" {
__declspec(dllexport) DWORD WINAPI bfl_LoadPlugins(LPVOID lpThreadParameter);
__declspec(dllexport) DWORD WINAPI bfl_UnloadPlugins(LPVOID lpThreadParameter);
__declspec(dllexport) DWORD WINAPI bfl_MessagePlugin(LPVOID lpThreadParameter);
}

namespace {

/// This is a singleton instance - there will only be 1 bifrost_loader.dll per process. This class holds information about all the loaded plugins and knows how
/// to interact with them.
class LoaderContext {
 public:
  struct Storage {
    ~Storage() {
      // Remove shared memory logger
      Context->SetLogger(BufferedLogger.get());
      this->SharedLogger.reset();
      this->ModuleLoader.reset();

      // Disconnect memory
      this->Memory.reset();
      this->Context.reset();
    }

    std::unique_ptr<BufferedLogger> BufferedLogger;
    std::unique_ptr<Context> Context;
    std::unique_ptr<SharedMemory> Memory;
    std::unique_ptr<SharedLogger> SharedLogger;
    std::unique_ptr<ModuleLoader> ModuleLoader;
  };

  struct Plugin {
    HMODULE Handle;
  };

  LoaderContext(LPVOID lpThreadParameter) { m_storage = InitStorage(lpThreadParameter); }

  ~LoaderContext() {
    try {
      if (m_storage) {
        SafeExec(m_storage.get(), "library tear down", [&]() { UnloadPluginsImpl(m_storage->Context.get(), {{}, true}); });
      }
    } catch (...) {
    }
  }

  /// Parse InjectorParam given by `lpThreadParameter` and setup context and shared memory
  std::unique_ptr<Storage> InitStorage(LPVOID lpThreadParameter) {
    auto storage = std::make_unique<Storage>();
    SafeExec(storage.get(), "library initialization", [&lpThreadParameter, &storage, this]() {
      storage->BufferedLogger = std::make_unique<BufferedLogger>();

      // Set up a buffered logger
      storage->Context = std::make_unique<Context>();
      storage->Context->SetLogger(storage->BufferedLogger.get());

      storage->ModuleLoader = std::make_unique<ModuleLoader>(storage->Context.get());
      auto curModule = WStringToString(storage->ModuleLoader->GetCurrentModuleName());
      storage->BufferedLogger->SetModule(curModule.c_str());

      // Try to unpack the injector params
      auto param = InjectorParam::Deserialize(storage->Context.get(), (const char*)lpThreadParameter);

      // Connect to the shared memory
      storage->Memory = std::make_unique<SharedMemory>(storage->Context.get(), param.SharedMemoryName, param.SharedMemorySize);
      storage->Context->SetMemory(storage->Memory.get());

      // Flush the buffered logger and start logging to shared memory
      storage->SharedLogger = std::make_unique<SharedLogger>(storage->Context.get());
      storage->SharedLogger->SetModule(curModule.c_str());
      storage->Context->SetLogger(storage->SharedLogger.get());
      storage->BufferedLogger->Flush(storage->SharedLogger.get());
    });
    return storage;
  }

  bool LoadPluginImpl(Context* ctx, const PluginLoadParam::Plugin& p) {
    auto it = m_pluginMap.find(p.Identifier);
    bool loaded = it != m_pluginMap.end();

    if (p.ForceLoad && loaded) {
      UnloadPluginsImpl(ctx, PluginUnloadParam{{p.Identifier}, false});
    } else if (loaded) {
      ctx->Logger().InfoFormat("Plugin \"%s\" has already been loaded. Skipping.", p.Identifier);
      return true;
    }

    // Load the library
    HMODULE handle = m_storage->ModuleLoader->GetOrLoadModule(p.Identifier, {p.Path});
    BIFROST_CHECK_WIN_CALL_CTX(ctx, ::DisableThreadLibraryCalls(handle) != 0);

    // Get the init procedure
    auto bifrost_PluginSetUp = (BIFROST_PLUGIN_SETUP_PROC_TYPE)::GetProcAddress(handle, BIFROST_PLUGIN_SETUP_PROC_NAME_STRING);
    if (!bifrost_PluginSetUp) {
      ctx->Logger().WarnFormat("Failed to load plugin \"%s\": Failed to get set up procedure \"%s\": %s", p.Identifier, BIFROST_PLUGIN_SETUP_PROC_NAME_STRING,
                               GetLastWin32Error().c_str());
      return true;
    }

    PluginContext::SetUpParam param;
    param.SharedMemoryName = m_storage->Memory->GetName();
    param.SharedMemorySize = m_storage->Memory->GetSizeInBytes();
    param.Arguments = p.Arguments;

    bool success = bifrost_PluginSetUp((void*)&param) == 0;
    if (success) {
      // Add the plugin to loaded plugin map
      m_pluginMap.emplace(p.Identifier, Plugin{handle});
    }
    return success;
  }

  DWORD LoadPlugins(LPVOID lpThreadParameter) {
    bool success;
    SafeExec(m_storage.get(), "loading", [&lpThreadParameter, &success, this]() {
      auto ctx = m_storage->Context.get();
      auto param = CheckSharedMemory(lpThreadParameter);
      auto loadParam = PluginLoadParam::Deserialize(param.CustomArgument);

      for (const auto& p : loadParam.Plugins) success &= LoadPluginImpl(ctx, p);
    });
    return success;
  }

  bool UnloadPluginImpl(Context* ctx, std::string identifier, const Plugin& p) {
    // Get the tear-down procedure
    auto bifrost_PluginTearDown = (BIFROST_PLUGIN_TEARDOWN_PROC_TYPE)::GetProcAddress(p.Handle, BIFROST_PLUGIN_TEARDOWN_PROC_NAME_STRING);
    if (!bifrost_PluginTearDown) {
      ctx->Logger().WarnFormat("Failed to unload plugin \"%s\": Failed to get tear down procedure \"%s\": %s", identifier,
                               BIFROST_PLUGIN_TEARDOWN_PROC_NAME_STRING, GetLastWin32Error().c_str());
      return true;
    }

    PluginContext::TearDownParam param;
    param.NoFail = false;
    bool success = bifrost_PluginTearDown((void*)&param) == 0;
    if (success) {
      // Remove the plugin from the loaded plugin map
      m_pluginMap.erase(identifier);
    }
    return success;
  }

  bool UnloadPluginsImpl(Context* ctx, const PluginUnloadParam& p) {
    std::vector<std::pair<std::string, Plugin>> pluginsToUnload;
    bool success = true;

    if (p.UnloadAll) {
      for (const auto& pair : m_pluginMap) {
        pluginsToUnload.emplace_back(pair);
      }
    } else {
      for (const auto& plugin : p.Plugins) {
        auto it = m_pluginMap.find(plugin);

        // Check if the plugin is loaded
        if (it != m_pluginMap.end()) {
          pluginsToUnload.emplace_back(plugin, it->second);
        } else {
          ctx->Logger().WarnFormat("Failed to unload plugin \"%s\": plugin is not loaded", plugin);
        }
      }
    }

    for (const auto& plugin : pluginsToUnload) success &= UnloadPluginImpl(ctx, plugin.first, plugin.second);
    return success;
  }

  DWORD UnloadPlugins(LPVOID lpThreadParameter) {
    bool success;
    SafeExec(m_storage.get(), "unloading", [&lpThreadParameter, &success, this]() {
      auto ctx = m_storage->Context.get();
      auto param = CheckSharedMemory(lpThreadParameter);
      auto unloadParam = PluginUnloadParam::Deserialize(param.CustomArgument);

      success &= UnloadPluginsImpl(ctx, unloadParam);
    });
    return success;
  }

  DWORD MessagePlugin(LPVOID lpThreadParameter) { return 0; }

  /// Check the requested shared memory is the same as the used shared memory by deserializing InjectorParam
  InjectorParam CheckSharedMemory(LPVOID lpThreadParameter) {
    auto ctx = m_storage->Context.get();

    auto param = InjectorParam::Deserialize(ctx, (const char*)lpThreadParameter);

    // Empty shared memory provided -> skip it
    if (param.SharedMemoryName.empty() && param.SharedMemoryName != ctx->Memory().GetName()) {
      ctx->Logger().WarnFormat("Empty shared memory provided, using existing shared memory \"%s\"", ctx->Memory().GetName());
    }

    // The shared memories are different -> abort
    if (param.SharedMemoryName != ctx->Memory().GetName()) {
      auto otherStorage = InitStorage(lpThreadParameter);
      SafeExec(otherStorage.get(), "shared memory check", [&]() {
        throw Exception("Provided shared memory \"%s\" differs from the already existing bifrost_loader shared memory in target process \"%s\"",
                        otherStorage->Context->Memory().GetName(), ctx->Memory().GetName());
      });
    }

    return param;
  }

  /// Safely execute function `func`
  template <class FuncT>
  inline void SafeExec(Storage* storage, const char* action, FuncT&& func) {
    try {
      func();
    } catch (std::exception& e) {
      HandleException(storage, action, &e);
      throw;
    } catch (...) {
      HandleException(storage, action, nullptr);
      throw;
    }
  }

  /// Is the context properly initialized?
  inline bool IsInitialized() { return m_storage != nullptr; }

  /// Try very hard to get a logging message to the user
  void HandleException(Storage* storage, const char* action, std::exception* ep) {
    auto log = [&](ILogger* logger) {
      if (ep) {
        logger->ErrorFormat("Plugin %s failed: %s", action, ep->what());
      } else {
        logger->Error(BIFROST_API_UNCAUGHT_EXCEPTION);
      }
    };

    if (storage->SharedLogger) {
      log(storage->SharedLogger.get());
    } else if (storage->BufferedLogger) {
      log(storage->BufferedLogger.get());
      storage->BufferedLogger->FlushToDisk("log.bifrost_loader.txt");
      storage->BufferedLogger->FlushToErr();
    }
  }

  Storage* GetStorage() const { return m_storage.get(); }

 private:
  std::unique_ptr<Storage> m_storage;
  std::unordered_map<std::string, Plugin> m_pluginMap;
};

// Singleton context
static SpinMutex g_mutex;
static LoaderContext* g_context = nullptr;

void FreeLoaderContext(void) {
  if (g_context) {
    delete g_context;
    g_context = nullptr;
  }
}

}  // namespace

#define BFL_FUNC(stmt)                                    \
  try {                                                   \
    {                                                     \
      BIFROST_LOCK_GUARD(g_mutex);                        \
      if (!g_context) {                                   \
        g_context = new LoaderContext(lpThreadParameter); \
        std::atexit(FreeLoaderContext);                   \
      }                                                   \
    }                                                     \
    stmt                                                  \
  } catch (...) {                                         \
    return 1;                                             \
  }                                                       \
  return 0;

DWORD WINAPI bfl_LoadPlugins(LPVOID lpThreadParameter) {
  BFL_FUNC({ return g_context->LoadPlugins(lpThreadParameter); });
}

DWORD WINAPI bfl_UnloadPlugins(LPVOID lpThreadParameter) {
  BFL_FUNC({ return g_context->UnloadPlugins(lpThreadParameter); });
}

DWORD WINAPI bfl_MessagePlugin(LPVOID lpThreadParameter) {
  BFL_FUNC({ return g_context->MessagePlugin(lpThreadParameter); });
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) { return TRUE; }