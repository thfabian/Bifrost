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

/// Try very hard to get a logging message to the user
void HandleException(BufferedLogger* bufferedLogger, SharedLogger* sharedLogger, std::exception* ep) {
  auto log = [&](ILogger* logger) {
    if (ep) {
      logger->ErrorFormat("Plugin loading failed: %s", ep->what());
    } else {
      logger->Error(BIFROST_API_UNCAUGHT_EXCEPTION);
    }
  };

  if (sharedLogger) {
    log(sharedLogger);
  } else if (bufferedLogger) {
    log(bufferedLogger);
    if (!bufferedLogger->FlushToDisk("bifrost_loader.log.txt")) bufferedLogger->FlushToErr();
  }
}

}  // namespace

DWORD WINAPI bfl_LoadPlugins(LPVOID lpThreadParameter) {
  std::unique_ptr<Context> ctx;
  std::unique_ptr<SharedMemory> memory;

  std::unique_ptr<BufferedLogger> bufferedLogger;
  std::unique_ptr<SharedLogger> sharedLogger;

  try {
    bufferedLogger = std::make_unique<BufferedLogger>();

    // Set up a buffered logger
    ctx = std::make_unique<Context>();
    ctx->SetLogger(bufferedLogger.get());

    ModuleLoader moduleLoader(ctx.get());
    auto curModule = WStringToString(moduleLoader.GetCurrentModuleName());
    bufferedLogger->SetModule(curModule.c_str());

    // Try to unpack the injector params
    auto param = InjectorParam::Deserialize(ctx.get(), (const char*)lpThreadParameter);

    // Connect to the shared memory
    memory = std::make_unique<SharedMemory>(ctx.get(), param.SharedMemoryName, param.SharedMemorySize);
    ctx->SetMemory(memory.get());

    // Flush the buffered logger and start logging to shared memory
    sharedLogger = std::make_unique<SharedLogger>(ctx.get());
    sharedLogger->SetModule(curModule.c_str());
    ctx->SetLogger(sharedLogger.get());
    bufferedLogger->Flush(sharedLogger.get());

    // Remove shared memory logger
    ctx->SetLogger(bufferedLogger.get());
    sharedLogger = nullptr;

    // Disconnect memory
    memory.reset();
    ctx.reset();

  } catch (std::exception& e) {
    HandleException(bufferedLogger.get(), sharedLogger.get(), &e);
    return 1;
  } catch (...) {
    HandleException(bufferedLogger.get(), sharedLogger.get(), nullptr);
    return 1;
  }
  return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) { return TRUE; }