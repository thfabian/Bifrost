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
#include "bifrost/core/buffered_logger.h"
#include "bifrost/core/context.h"
#include "bifrost/core/injector_param.h"
#include "bifrost/core/shared_memory.h"
#include "bifrost/core/sm_log_stash.h"
#include "bifrost/core/module_loader.h"

using namespace bifrost;

extern "C" {
__declspec(dllexport) DWORD WINAPI bfl_LoadPlugins(LPVOID lpThreadParameter);
__declspec(dllexport) DWORD WINAPI bfl_UnloadPlugins(LPVOID lpThreadParameter);
__declspec(dllexport) DWORD WINAPI bfl_MessagePlugin(LPVOID lpThreadParameter);
}

namespace {

class SharedLogger : public ILogger {
 public:
  SharedLogger(Context* ctx) : m_ctx(ctx) {}

  virtual void SetModule(const char* module) override { m_module = module; }
  virtual void Sink(LogLevel level, const char* module, const char* msg) override {
    m_ctx->Memory().GetSMLogStash()->Push(m_ctx, static_cast<u32>(level), module, msg);
  }
  virtual void Sink(LogLevel level, const char* msg) override { Sink(level, m_module.c_str(), msg); }

 private:
  Context* m_ctx;
  std::string m_module;
};

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

  LoaderContext(LPVOID lpThreadParameter) { m_storage = InitStorage(lpThreadParameter); }

  /// Parse InjectorParam given by `lpThreadParameter` and setup context and shared memory
  std::unique_ptr<Storage> InitStorage(LPVOID lpThreadParameter) {
    auto storage = std::make_unique<Storage>();
    SafeExec(storage.get(), [&lpThreadParameter, &storage, this]() {
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

  /// Safely execute function `func`
  template <class FuncT>
  inline void SafeExec(Storage* storage, FuncT&& func) {
    try {
      func();
    } catch (std::exception& e) {
      HandleException(storage, &e);
      throw;
    } catch (...) {
      HandleException(storage, nullptr);
      throw;
    }
  }

  /// Is the context properly initialized?
  inline bool IsInitialized() { return m_storage != nullptr; }

  /// Try very hard to get a logging message to the user
  void HandleException(Storage* storage, std::exception* ep) {
    auto log = [&](ILogger* logger) {
      if (ep) {
        logger->ErrorFormat("Plugin loading failed: %s", ep->what());
      } else {
        logger->Error(BIFROST_API_UNCAUGHT_EXCEPTION);
      }
    };

    if (storage->SharedLogger) {
      log(storage->SharedLogger.get());
    } else if (storage->BufferedLogger) {
      log(storage->BufferedLogger.get());
      storage->BufferedLogger->FlushToDisk("bifrost_loader.log.txt");
      storage->BufferedLogger->FlushToErr();
    }
  }

 private:
  std::unique_ptr<Storage> m_storage;
};

// Singleton context
static SpinMutex g_mutex;
static LoaderContext* g_context = nullptr;

}  // namespace

#define BFL_FUNC(stmt)                                                  \
  try {                                                                 \
    {                                                                   \
      BIFROST_LOCK_GUARD(g_mutex);                                      \
      if (!g_context) g_context = new LoaderContext(lpThreadParameter); \
    }                                                                   \
    stmt                                                                \
  } catch (...) {                                                       \
    return 1;                                                           \
  }                                                                     \
  return 0;

DWORD WINAPI bfl_LoadPlugins(LPVOID lpThreadParameter) {
  BFL_FUNC({

  });
}

DWORD WINAPI bfl_UnloadPlugins(LPVOID lpThreadParameter) {
  BFL_FUNC({

  });
}

DWORD WINAPI bfl_MessagePlugin(LPVOID lpThreadParameter) {
  BFL_FUNC({

  });
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) { return TRUE; }