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

#include "bifrost/api/injector.h"
#include "bifrost/api/helper.h"
#include "bifrost/core/buffered_logger.h"
#include "bifrost/core/context.h"
#include "bifrost/core/error.h"
#include "bifrost/core/exception.h"
#include "bifrost/core/injector_param.h"
#include "bifrost/core/macros.h"
#include "bifrost/core/module_loader.h"
#include "bifrost/core/plugin_loader.h"
#include "bifrost/core/process.h"
#include "bifrost/core/shared_memory.h"
#include "bifrost/core/sm_log_stash.h"

using namespace bifrost;
using namespace bifrost::api;

namespace {

#define BIFROST_INJECTOR_CATCH_ALL(stmts) BIFROST_API_CATCH_ALL_IMPL(stmts, BFI_ERROR)
#define BIFROST_INJECTOR_CATCH_ALL_PTR(stmts) BIFROST_API_CATCH_ALL_IMPL(stmts, nullptr)

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
    m_loader = std::make_unique<ModuleLoader>(m_ctx.get());
    m_bufferedLogger->SetModule(WStringToString(m_loader->GetCurrentModuleName()).c_str());

    SetUpBufferedLogger();
  }

  ~InjectorContext() {
    m_logStashConsumer.reset();
    m_memory.reset();
    m_loader.reset();
    m_bufferedLogger.reset();
    m_forwardLogger.reset();
  }

  // Inject the plugins
  bfi_Status ProcessInject(const bfi_InjectorArguments* args, bfi_Process_t** process) {
    m_ctx->Logger().Info("Injecting plugins ...");

    std::unique_ptr<Process> proc = nullptr;
    try {
      // Create shared memory
      auto uuid = UUID(m_ctx.get());
      m_memory = std::make_unique<SharedMemory>(m_ctx.get(), uuid, args->SharedMemorySizeInBytes);
      m_ctx->SetMemory(m_memory.get());
      SetUpLogConsumer();

      // Write plugins to shared memory
      std::vector<Plugin> plugins;
      for (u32 i = 0; i < args->NumPlugins; ++i) {
        plugins.emplace_back(Plugin{args->Plugins[i].Path, args->Plugins[i].Arguments ? args->Plugins[i].Arguments : ""});
      }
      PluginLoader loader(m_ctx.get());
      loader.Serialize(plugins);

      // Setup the injector arguments for bifrost_loader.dll
      InjectorParam param;
      param.Pid = ::GetCurrentProcessId();
      param.SharedMemoryName = m_ctx->Memory().GetName();
      param.SharedMemorySize = (u32)m_ctx->Memory().GetSizeInBytes();

      std::wstring cwd(2 * MAX_PATH, '\0');
      DWORD len = (u32)cwd.size();
      BIFROST_ASSERT_CALL_CTX(m_ctx.get(), (len = ::GetCurrentDirectory(len, (wchar_t*)cwd.data())) != 0);
      param.WorkingDirectory = cwd.substr(0, len);
      m_ctx->Logger().DebugFormat(L"Using working directory: \"%s\"", param.WorkingDirectory.c_str());

      // Prepare bifrost_loader.dll to be injected
      Process::InjectArguments injectArguments;
      injectArguments.DllPath = m_loader->GetModulePath(m_loader->GetModule(L"bifrost_loader.dll"));
      injectArguments.InitProcArg = param.Serialize();
      injectArguments.InitProcName = "bfl_LoadPlugins";
      injectArguments.TimeoutInMs = args->InjectorTimeoutInS * 1000;

      // Launch the process and perform injection
      switch (args->Mode) {
        case BFI_LAUNCH: {
          Process::LaunchArguments arguments{args->Executable ? args->Executable : L"", args->Arguments ? args->Arguments : "", true};
          proc = std::make_unique<Process>(m_ctx.get(), std::move(arguments));
          break;
        }
        case BFI_CONNECT_VIA_PID: {
          proc = std::make_unique<Process>(m_ctx.get(), args->Pid);
          break;
        }
        case BFI_CONNECT_VIA_NAME: {
          proc = std::make_unique<Process>(m_ctx.get(), args->Name);
          break;
        }
        case BFI_UNKNOWN:
        default:
          throw Exception("Failed to inject executabled: Unknown executable mode (bfi_InjectorArguments_t::Mode is set to BFI_UNKNOWN)");
      }
      proc->Inject(std::move(injectArguments));
      if (args->Mode == BFI_LAUNCH) proc->Resume();

    } catch (...) {
      if (proc) {
        KillProcess(m_ctx.get(), proc->GetPid());
      }
      m_ctx->Logger().Error("Failed to inject plugins");
      throw;
    }

    *process = new bfi_Process;
    (*process)->_Internal = proc.release();

    m_ctx->Logger().Info("Successfully injected plugins");
    return BFI_OK;
  }

  // Wait for the process to complete, kill it if we time out
  bfi_Status ProcessWait(Process* process, uint32_t timeout, int32_t* exitCode) {
    m_ctx->Logger().InfoFormat("Waiting for %s seconds for process to complete ...", timeout == 0 ? "infinite" : std::to_string(timeout).c_str());
    u32 reason = process->Wait(timeout);

    if (reason == WAIT_TIMEOUT || reason == WAIT_ABANDONED) {
      m_ctx->Logger().Warn("Process timed out or wait was abandoned. Killing process ...");
      KillProcess(m_ctx.get(), process->GetPid());
    }

    if (exitCode) {
      const u32* ec = process->GetExitCode();
      *exitCode = ec ? *ec : STILL_ACTIVE;
    }
    return BFI_OK;
  }

  // Poll the process and set the error code if it completed
  bfi_Status ProcessPoll(Process* process, int32_t* running, int32_t* exitCode) { return BFI_OK; }

  // Kill the process
  bfi_Status ProcessKill(Process* process) { return BFI_OK; }

  // Error stash
  void SetLastError(std::string msg) { m_error = std::move(msg); }
  const char* GetLastError() { return m_error.empty() ? "No Error" : m_error.c_str(); }

  // Set the log callback
  bfi_Status SetLogCallback(bfi_LoggingCallback cb) {
    if (cb != NULL) {
      SetUpForwardLogger(cb);
    } else {
      SetUpBufferedLogger();
    }
    return BFI_OK;
  }

  void SetUpBufferedLogger() { m_ctx->SetLogger(m_bufferedLogger.get()); }

  void SetUpForwardLogger(bfi_LoggingCallback cb) {
    m_forwardLogger = std::make_unique<ForwardLogger>(cb);
    m_forwardLogger->SetModule(m_bufferedLogger->GetModule());
    m_ctx->SetLogger(m_forwardLogger.get());
    m_bufferedLogger->Flush(m_forwardLogger.get());
  }

  void SetUpLogConsumer() {
    if (m_memory) {
      m_logStashConsumer = std::make_unique<LogStashConsumer>(m_ctx.get(), m_memory->GetSMLogStash(), &m_ctx->Logger());
    }
  }

  Context* GetContext() { return m_ctx.get(); }

 private:
  std::string m_error;
  std::unique_ptr<Context> m_ctx;

  std::unique_ptr<SharedMemory> m_memory;
  std::unique_ptr<LogStashConsumer> m_logStashConsumer;
  std::unique_ptr<ModuleLoader> m_loader;

  std::unique_ptr<BufferedLogger> m_bufferedLogger;
  std::unique_ptr<ForwardLogger> m_forwardLogger;
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

BIFROST_INJECTOR_API bfi_Status bfi_ProcessKillByName(bfi_Context* ctx, const wchar_t* name) {
  BIFROST_INJECTOR_CATCH_ALL({
    KillProcess(Get(ctx)->GetContext(), name);
    return BFI_OK;
  })
}

#pragma endregion

#pragma region Injector

bfi_InjectorArguments* bfi_InjectorArgumentsInit(bfi_Context* ctx) {
  BIFROST_INJECTOR_CATCH_ALL_PTR({
    bfi_InjectorArguments* args = new bfi_InjectorArguments;
    ZeroMemory(args, sizeof(bfi_InjectorArguments));
    args->InjectorTimeoutInS = BIFROST_INJECTOR_DEFAULT_InjectorArguments_InjectorTimeoutInS;
    args->SharedMemorySizeInBytes = BIFROST_INJECTOR_DEFAULT_InjectorArguments_SharedMemorySizeInBytes;
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
