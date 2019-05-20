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

#include "bifrost/template/plugin_proc_decl.h"

#include "bifrost/api/helper.h"
#include "bifrost/api/plugin_impl.h"
#include "bifrost/core/buffered_logger.h"
#include "bifrost/core/context.h"
#include "bifrost/core/error.h"
#include "bifrost/core/exception.h"
#include "bifrost/core/injector_param.h"
#include "bifrost/core/module_loader.h"
#include "bifrost/core/plugin_param.h"
#include "bifrost/core/shared_memory.h"
#include "bifrost/core/sm_log_stash.h"

using namespace bifrost;
using namespace bifrost::api;

extern "C" {
__declspec(dllexport) DWORD WINAPI bfl_LoadPlugins(LPVOID lpThreadParameter);
__declspec(dllexport) DWORD WINAPI bfl_UnloadPlugins(LPVOID lpThreadParameter);
__declspec(dllexport) DWORD WINAPI bfl_MessagePlugin(LPVOID lpThreadParameter);
}

namespace {

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
    std::unique_ptr<PluginManager> PluginManager;
  };

  LoaderContext(LPVOID lpThreadParameter) { m_storage = InitStorage(lpThreadParameter); }

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

  void LoadPlugins(LPVOID lpThreadParameter) {
    SafeExec(m_storage.get(), "loading", [&lpThreadParameter, this]() {
      auto ctx = m_storage->Context.get();
      auto param = CheckSharedMemory(lpThreadParameter);
      auto loadParam = PluginLoadParam::Deserialize(param.CustomArgument);

      for (const auto& p : loadParam.Plugins) {

        // Load the library
        HMODULE handle = m_storage->ModuleLoader->GetOrLoadModule(p.Identifier, {p.Path});
        BIFROST_CHECK_WIN_CALL_CTX(ctx, ::DisableThreadLibraryCalls(handle) != 0);

        // Get the init procedure
        auto bifrost_PluginSetUp = (BIFROST_PLUGIN_SETUP_PROC_TYPE)::GetProcAddress(handle, BIFROST_PLUGIN_SETUP_PROC_NAME_STRING);
        if (!bifrost_PluginSetUp) {
          ctx->Logger().WarnFormat("Failed to load plugin \"%s\": Failed to get set up procedure \"%s\": %s", p.Identifier,
                                   BIFROST_PLUGIN_SETUP_PROC_NAME_STRING, GetLastWin32Error().c_str());
          continue;
        }


        bifrost_PluginSetUp(nullptr);
      }
    });
  }

  void UnloadPlugins(LPVOID lpThreadParameter) {
  
  }

  void MessagePlugin(LPVOID lpThreadParameter) {
  
  }

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
  BFL_FUNC({ g_context->LoadPlugins(lpThreadParameter); });
}

DWORD WINAPI bfl_UnloadPlugins(LPVOID lpThreadParameter) {
  BFL_FUNC({ g_context->UnloadPlugins(lpThreadParameter); });
}

DWORD WINAPI bfl_MessagePlugin(LPVOID lpThreadParameter) {
  BFL_FUNC({ g_context->MessagePlugin(lpThreadParameter); });
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) { return TRUE; }