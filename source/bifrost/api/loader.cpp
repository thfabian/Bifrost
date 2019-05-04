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

#include "bifrost/core/buffered_logger.h"
#include "bifrost/core/context.h"
#include "bifrost/core/injector_param.h"
#include "bifrost/core/shared_memory.h"
#include "bifrost/core/sm_log_stash.h"

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
  SpinMutex m_mutex;
  Context* m_ctx;
  std::string m_module;
};

}  // namespace

DWORD WINAPI bfl_LoadPlugins(LPVOID lpThreadParameter) {
  // using namespace bifrost;
  //
  // auto ctx = std::make_unique<Context>();
  // std::unique_ptr<LoggerBuffered> bufferedLogger = nullptr;

  // try {
  //  InjectorParams param((const char*)lpThreadParameter);

  //  // Create buffered logger
  //  bufferedLogger = std::make_unique<LoggerBuffered>(ctx.get(), param.LogFile());
  //  ctx->SetLogger(bufferedLogger.get());

  //  // Create shared memory
  //  auto mem = std::make_unique<SharedMemory>(ctx.get(), param.SmName(), param.SmSize());

  //  // Create shared logger
  //  auto sharedLogger = std::make_unique<LoggerShared>(ctx.get(), mem->GetSMLogStash());
  //  ctx->SetLogger(sharedLogger.get());
  //  bufferedLogger->Flush(sharedLogger.get());

  //} catch (std::exception& e) {
  //  if(bufferedLogger) {
  //    ctx->Logger().ErrorFormat("Uncaught exception: %s", e.what());
  //    bufferedLogger.FlushToDisk();
  //  }
  //  return 1;
  //}
  return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) { return TRUE; }