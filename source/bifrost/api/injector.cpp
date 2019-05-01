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

#include "bifrost/api/injector.h"
#include "bifrost/core/common.h"
#include "bifrost/core/context.h"
#include "bifrost/core/error.h"
#include "bifrost/core/macros.h"
#include "bifrost/core/buffered_logger.h"
#include "bifrost/core/injector_param.h"
#include "bifrost/core/shared_memory.h"
#include "bifrost/core/sm_log_stash.h"

using namespace bifrost;

#define BIFROST_INJECTOR_VERSION_MAJOR 0
#define BIFROST_INJECTOR_VERSION_MINOR 0
#define BIFROST_INJECTOR_VERSION_PATCH 1

#ifdef NDEBUG
#define BIFROST_INJECTOR_UNCAUGHT_EXCEPTION "Uncaught exception.\n  Function:" __FUNCTION__
#else
#define BIFROST_INJECTOR_UNCAUGHT_EXCEPTION "Uncaught exception.\n  Function: " __FUNCTION__ "\n  File: " __FILE__ ":" BIFROST_STRINGIFY(__LINE__)
#endif

#define BIFROST_INJECTOR_CATCH_ALL_IMPL(stmts, error)            \
  try {                                                          \
    stmts;                                                       \
  } catch (std::exception & e) {                                 \
    Get(ctx)->SetLastError(e.what());                            \
    return error;                                                \
  } catch (...) {                                                \
    Get(ctx)->SetLastError(BIFROST_INJECTOR_UNCAUGHT_EXCEPTION); \
    return error;                                                \
  }

#define BIFROST_INJECTOR_CATCH_ALL(stmts) BIFROST_INJECTOR_CATCH_ALL_IMPL(stmts, BFI_ERROR)
#define BIFROST_INJECTOR_CATCH_ALL_PTR(stmts) BIFROST_INJECTOR_CATCH_ALL_IMPL(stmts, nullptr)

namespace {

class ForwardLogger : public ILogger {
 public:
  ForwardLogger(bfi_LogCallback cb) : m_cb(cb) {}

  virtual void SetModule(const char* module) override { m_module = module; }
  virtual void Sink(LogLevel level, const char* module, const char* msg) override { m_cb(static_cast<uint32_t>(level), module, msg); }
  virtual void Sink(LogLevel level, const char* msg) override { Sink(level, m_module.c_str(), msg); }

 private:
  std::string m_module;
  bfi_LogCallback m_cb;
};

class InjectorContext {
 public:
  InjectorContext() {
    m_ctx = std::make_unique<Context>();
    m_bufferedLogger = std::make_unique<BufferedLogger>();
  }

  void SetLastError(std::string msg) { m_error = std::move(msg); }
  const char* GetLastError() { return m_error.empty() ? "No Error" : m_error.c_str(); }

  // Set the log callback
  bfi_Status SetLogCallback(bfi_LogCallback cb) {
    m_forwardLogger = std::make_unique<ForwardLogger>(cb);
    SetUpLogConsumer();
    m_bufferedLogger->Flush(m_forwardLogger.get());
    return BFI_OK;
  }

  // Get the currently active logger
  ILogger* GetCurrentLogger() {
    if (m_forwardLogger) return m_forwardLogger.get();
    return m_bufferedLogger.get();
  }

  // Set up the log consumer if we have an allocated shared memory
  void SetUpLogConsumer() {
    if (m_memory) {
      m_logStashConsumer = std::make_unique<LogStashConsumer>(m_ctx.get(), m_memory->GetSMLogStash(), GetCurrentLogger());
    }
  }

 private:
  std::string m_error;
  std::unique_ptr<Context> m_ctx;
  std::unique_ptr<SharedMemory> m_memory;

  std::unique_ptr<BufferedLogger> m_bufferedLogger;
  std::unique_ptr<ForwardLogger> m_forwardLogger;
  std::unique_ptr<LogStashConsumer> m_logStashConsumer;
};

InjectorContext* Get(bfi_Context* ctx) { return (InjectorContext*)ctx->_Pointer; }

}  // namespace

bfi_Context* bfi_Init() {
  bfi_Context* ctx = nullptr;
  try {
    ctx = new bfi_Context;
    ctx->_Pointer = new InjectorContext;
  } catch (...) {
  }
  return ctx;
}

void bfi_Free(bfi_Context* ctx) {
  if (ctx) {
    if (ctx->_Pointer) delete (InjectorContext*)ctx->_Pointer;
    ctx->_Pointer = nullptr;
    delete ctx;
  }
}

const char* bfi_GetLastError(bfi_Context* ctx) { return Get(ctx)->GetLastError(); }

bfi_Status bfi_SetCallback(bfi_Context* ctx, bfi_LogCallback cb){BIFROST_INJECTOR_CATCH_ALL({ return Get(ctx)->SetLogCallback(cb); })}

bfi_Version bfi_GetVersion() {
  return {BIFROST_INJECTOR_VERSION_MAJOR, BIFROST_INJECTOR_VERSION_MINOR, BIFROST_INJECTOR_VERSION_PATCH};
}

const char* bfi_GetVersionString() {
  return BIFROST_STRINGIFY(BIFROST_INJECTOR_VERSION_MAJOR) "." BIFROST_STRINGIFY(BIFROST_INJECTOR_VERSION_MINOR) "." BIFROST_STRINGIFY(
      BIFROST_INJECTOR_VERSION_PATCH);
}

bfi_InjectorArguments* bfi_InjectorArgumentsInit(bfi_Context* ctx) {
  BIFROST_INJECTOR_CATCH_ALL_PTR({
    bfi_InjectorArguments* args = new bfi_InjectorArguments;
    ZeroMemory(args, sizeof(bfi_InjectorArguments));
    args->InjectorTimeoutInMs = 5000;

    return args;
  });
}

bfi_Status bfi_Inject(bfi_Context* ctx, bfi_InjectorArguments* args) { return BFI_OK; }

bfi_Status bfi_InjectorArgumentsFree(bfi_Context* ctx, bfi_InjectorArguments* args) {
  BIFROST_INJECTOR_CATCH_ALL({
    if (args) delete args;
    return BFI_OK;
  });
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) { return TRUE; }
