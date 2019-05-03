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
#include "bifrost/core/process.h"
#include "bifrost/core/exception.h"
#include "bifrost/core/buffered_logger.h"
#include "bifrost/core/injector_param.h"
#include "bifrost/core/shared_memory.h"
#include "bifrost/core/sm_log_stash.h"

using namespace bifrost;

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

// Generic construction of a struct with an _Internal pointer
template <class StructT, class ClassT, class... ArgsT>
StructT* Init(ArgsT... args) {
  StructT* s = nullptr;
  try {
    s = new StructT;
    s->_Internal = new ClassT(std::forward<ArgsT>(args)...);
  } catch (...) {
  }
  return s;
}

// Generic destruction of a struct with an _Internal pointer
template <class StructT, class ClassT>
void Free(StructT* s) {
  if (s) {
    if (s->_Internal) delete (ClassT*)s->_Internal;
    s->_Internal = nullptr;
    delete s;
  }
}

class ForwardLogger : public ILogger {
 public:
  ForwardLogger(bfi_LoggingCallback cb) : m_cb(cb) {}

  virtual void SetModule(const char* module) override { m_module = module; }
  virtual void Sink(LogLevel level, const char* module, const char* msg) override { m_cb(static_cast<uint32_t>(level), module, msg); }
  virtual void Sink(LogLevel level, const char* msg) override { Sink(level, m_module.c_str(), msg); }

 private:
  std::string m_module;
  bfi_LoggingCallback m_cb;
};

class InjectorContext {
 public:
  InjectorContext() {
    m_ctx = std::make_unique<Context>();
    m_bufferedLogger = std::make_unique<BufferedLogger>();
  }

  // Inject the plugins
  bfi_Status ProcessInject(const bfi_InjectorArguments* args, bfi_Process_t** process) {
    
    // Generate UUID string for memory
    //RPC_STATUS errorCode = 0;
    UUID uuid;

    //if ((ec = ::UuidCreate(&uuid)) != RPC_S_OK) {
    //  throw Exception("Failed to get UUID - UuidCreate failed: %u", ec);
    //}

    
    return BFI_OK; 
  
  }

  // Wait for the process to complete, kill it if we time out
  bfi_Status ProcessWait(Process* process, uint32_t timeout, int32_t* exitCode) { return BFI_OK; }

  // Poll the process and set the error code if it completed
  bfi_Status ProcessPoll(Process* process, int32_t* running, int32_t* exitCode) { return BFI_OK; }

  // Kill the process
  bfi_Status ProcessKill(Process* process) { return BFI_OK; }

  // Error stash
  void SetLastError(std::string msg) { m_error = std::move(msg); }
  const char* GetLastError() { return m_error.empty() ? "No Error" : m_error.c_str(); }

  // Set the log callback
  bfi_Status SetLogCallback(bfi_LoggingCallback cb) {
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

InjectorContext* Get(bfi_Context* ctx) { return (InjectorContext*)ctx->_Internal; }
Process* Get(bfi_Process* process) { return (Process*)process->_Internal; }

}  // namespace

#pragma region Version

bfi_Version bfi_GetVersion() { return {BIFROST_INJECTOR_VERSION_MAJOR, BIFROST_INJECTOR_VERSION_MINOR, BIFROST_INJECTOR_VERSION_PATCH}; }

const char* bfi_GetVersionString() {
  return BIFROST_STRINGIFY(BIFROST_INJECTOR_VERSION_MAJOR) "." BIFROST_STRINGIFY(BIFROST_INJECTOR_VERSION_MINOR) "." BIFROST_STRINGIFY(
      BIFROST_INJECTOR_VERSION_PATCH);
}

#pragma endregion

#pragma region Context

bfi_Context* bfi_ContextInit() { return Init<bfi_Context, InjectorContext>(); }

void bfi_ContextFree(bfi_Context* ctx) { Free<bfi_Context, InjectorContext>(ctx); }

const char* bfi_ContextGetLastError(bfi_Context* ctx) { return Get(ctx)->GetLastError(); }

bfi_Status bfi_ContextSetLoggingCallback(bfi_Context* ctx, bfi_LoggingCallback cb) {
  BIFROST_INJECTOR_CATCH_ALL({ return Get(ctx)->SetLogCallback(cb); })
}

#pragma endregion

#pragma region Process

BIFROST_INJECTOR_API bfi_Status bfi_ProcessFree(bfi_Context* ctx, bfi_Process* process) {
  Free<bfi_Process, Process>(process);
  return BFI_OK;
}

BIFROST_INJECTOR_API bfi_Status bfi_ProcessWait(bfi_Context* ctx, bfi_Process* process, int32_t timeout, int32_t* exitCode) {
  BIFROST_INJECTOR_CATCH_ALL({ return Get(ctx)->ProcessWait(Get(process), timeout, exitCode); });
}

BIFROST_INJECTOR_API bfi_Status bfi_ProcessPoll(bfi_Context* ctx, bfi_Process* process, int32_t* running, int32_t* exitCode) {
  BIFROST_INJECTOR_CATCH_ALL({ return Get(ctx)->ProcessPoll(Get(process), running, exitCode); });
}

BIFROST_INJECTOR_API bfi_Status bfi_ProcessKill(bfi_Context* ctx, bfi_Process* process) {
  BIFROST_INJECTOR_CATCH_ALL({ return Get(ctx)->ProcessKill(Get(process)); });
}

#pragma endregion

#pragma region Injector

bfi_InjectorArguments* bfi_InjectorArgumentsInit(bfi_Context* ctx) {
  BIFROST_INJECTOR_CATCH_ALL_PTR({
    bfi_InjectorArguments* args = new bfi_InjectorArguments;
    ZeroMemory(args, sizeof(bfi_InjectorArguments));
    args->InjectorTimeoutInS = 5;
    return args;
  });
}

bfi_Status bfi_InjectorArgumentsFree(bfi_Context* ctx, bfi_InjectorArguments* args) {
  BIFROST_INJECTOR_CATCH_ALL({
    if (args) delete args;
    return BFI_OK;
  });
}

bfi_Status bfi_ProcessInject(bfi_Context* ctx, const bfi_InjectorArguments* args, bfi_Process_t** process) {
  BIFROST_INJECTOR_CATCH_ALL({ return Get(ctx)->ProcessInject(args, process); });
}

#pragma endregion

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) { return TRUE; }
