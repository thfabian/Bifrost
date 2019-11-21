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
#include "bifrost/core/plugin_param.h"
#include "bifrost/core/process.h"
#include "bifrost/core/shared_memory.h"
#include "bifrost/core/sm_log_stash.h"
#include "bifrost/debugger/debugger.h"

#include "bifrost/template/plugin_fwd.h"

using namespace bifrost;
using namespace bifrost::api;

namespace {

#define BIFROST_INJECTOR_CATCH_ALL(stmts) BIFROST_API_CATCH_ALL_IMPL(ctx, stmts, BFP_ERROR)
#define BIFROST_INJECTOR_CATCH_ALL_PTR(stmts) BIFROST_API_CATCH_ALL_IMPL(ctx, stmts, nullptr)

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

  // Load the plugins
  bfi_Status PluginLoad(const bfi_PluginLoadArguments* args, bfi_Process_t** process, bfi_PluginLoadResult** result) {
    m_ctx->Logger().Info("Loading plugins into remote process ...");

    if (!args->Executable) throw Exception("bfi_PluginLoadArguments.Executable is NULL");
    if (!args->InjectorArguments) throw Exception("bfi_PluginLoadArguments.InjectorArguments is NULL");

    *process = NULL;
    *result = NULL;

    std::string smName;
    u32 smSize = 0, rpPid = 0;

    std::unique_ptr<Process> proc = nullptr;
    try {
      // Create or reuse the existing shared memory
      CreateOrReuseExistingSharedMemory(args->InjectorArguments->SharedMemoryName, args->InjectorArguments->SharedMemorySizeInBytes);

      // Setup the injector arguments for bifrost_loader.dll
      PluginLoadParam loadParam;
      for (u32 i = 0; i < args->NumPlugins; ++i) {
        loadParam.Plugins.emplace_back(
            PluginLoadParam::Plugin{args->Plugins[i].Name ? args->Plugins[i].Name : "", args->Plugins[i].Path ? args->Plugins[i].Path : L"",
                                    args->Plugins[i].Arguments ? args->Plugins[i].Arguments : "", static_cast<bool>(args->Plugins[i].ForceLoad)});
      }
      auto param = MakeInjectorParam(loadParam.Serialize());

      // Prepare bifrost_loader.dll to be injected
      Process::InjectArguments injectArguments;
      injectArguments.DllPath = GetPathOfBifrostLoader();
      injectArguments.InitProcArg = param.Serialize();
      injectArguments.InitProcName = "bfl_LoadPlugins";
      injectArguments.TimeoutInMs = args->InjectorArguments->TimeoutInS * 1000;

      // Launch the process and perform injection
			std::vector<u32> threads;
      switch (args->Executable->Mode) {
        case BFI_LAUNCH: {
          Process::LaunchArguments arguments{args->Executable->ExecutablePath ? args->Executable->ExecutablePath : L"",
                                             args->Executable->ExecutableArguments ? args->Executable->ExecutableArguments : "", true};
          proc = std::make_unique<Process>(m_ctx.get(), std::move(arguments));
					threads = proc->GetThreads(); // get the main thread
          break;
        }
        case BFI_CONNECT_VIA_PID: {
          proc = std::make_unique<Process>(m_ctx.get(), args->Executable->Pid);
          break;
        }
        case BFI_CONNECT_VIA_NAME: {
          proc = std::make_unique<Process>(m_ctx.get(), args->Executable->Name);
          break;
        }
        case BFI_UNKNOWN:
        default:
          throw Exception("Failed to inject executable: Unknown executable mode (bfi_InjectorArguments.Executable.Mode is set to BFI_UNKNOWN)");
      }
      rpPid = proc->GetPid();

      // Attach a debugger?
      if (args->InjectorArguments->Debugger) AttachDebugger(args->InjectorArguments, proc.get(), injectArguments);

      // Inject and load the plugins
      proc->Inject(std::move(injectArguments));

      // Resume execution of all threads..
      if (args->Executable->Mode == BFI_LAUNCH) proc->Resume(threads);

    } catch (...) {
      if (proc && args->Executable->Mode == BFI_LAUNCH) {
        KillProcess(m_ctx.get(), proc->GetPid());
      }

      m_ctx->Logger().Error("Failed to load plugins");
      throw;
    }

    *process = new bfi_Process;
    (*process)->_Internal = proc.release();

    // Assemble the result
    *result = new bfi_PluginLoadResult;
    (*result)->RemoteProcessPid = rpPid;
    (*result)->SharedMemorySize = smSize;
    (*result)->SharedMemoryName = new char[smName.size() + 1];
    std::memcpy((void*)(*result)->SharedMemoryName, smName.c_str(), smName.size() + 1);

    m_ctx->Logger().Info("Successfully loaded plugins");
    return BFP_OK;
  }

  // Unload the plugins
  bfi_Status PluginUnload(const bfi_PluginUnloadArguments* args, Process* process, bfi_PluginUnloadResult** result) {
    m_ctx->Logger().Info("Unloading plugins from remote process ...");

    if (!args->InjectorArguments) throw Exception("bfi_PluginUnloadArguments.InjectorArguments is NULL");

    *result = NULL;
    try {
      // Create or reuse the existing shared memory
      CreateOrReuseExistingSharedMemory(args->InjectorArguments->SharedMemoryName, args->InjectorArguments->SharedMemorySizeInBytes);

      // Setup the injector arguments for bifrost_loader.dll
      PluginUnloadParam loadParam;
      loadParam.UnloadAll = args->UnloadAll;
      if (!loadParam.UnloadAll) {
        for (u32 i = 0; i < args->NumPlugins; ++i) loadParam.Plugins.emplace_back(args->Plugins[i].Name);
      }
      auto param = MakeInjectorParam(loadParam.Serialize());

      // Prepare bifrost_loader.dll to be injected
      Process::InjectArguments injectArguments;
      injectArguments.DllPath = GetPathOfBifrostLoader();
      injectArguments.InitProcArg = param.Serialize();
      injectArguments.InitProcName = "bfl_UnloadPlugins";
      injectArguments.TimeoutInMs = args->InjectorArguments->TimeoutInS * 1000;

      // Attach a debugger?
      if (args->InjectorArguments->Debugger) AttachDebugger(args->InjectorArguments, process, injectArguments);

      // Inject and unload the plugins
      process->Inject(std::move(injectArguments));

    } catch (...) {
      m_ctx->Logger().Error("Failed to unload plugins");
      throw;
    }

    // Assemble the result
    *result = new bfi_PluginUnloadResult;
    (*result)->Unloaded = new int32_t[args->NumPlugins];
    std::memset((*result)->Unloaded, 1, args->NumPlugins * sizeof(int32_t));

    // TODO: Get a way to check if the plugin has been unloaded (requires communication back)

    m_ctx->Logger().Info("Successfully unloaded plugins");
    return BFP_OK;
  }

  // Get help message of the plugin
  bfi_Status PluginHelp(const wchar_t* path, char** help) {
    const char* helpStr = "";

    auto handle = m_loader->GetOrLoadModule(std::filesystem::path(path).filename().string(), {path});
    auto bifrost_PluginHelp = (BIFROST_PLUGIN_HELP_PROC_TYPE)::GetProcAddress(handle, BIFROST_PLUGIN_HELP_PROC_NAME_STRING);
    if (!bifrost_PluginHelp) {
      m_ctx->Logger().WarnFormat("Failed to get help of plugin \"%s\": failed to get help procedure \"%s\": %s", WStringToString(path),
                                 BIFROST_PLUGIN_HELP_PROC_NAME_STRING, GetLastWin32Error().c_str());
    } else {
      helpStr = bifrost_PluginHelp();
    }

    // Make a copy of the memory so we can deallocate it again
    std::size_t len = std::strlen(helpStr);
    (*help) = new char[len + 1];
    std::memcpy((*help), helpStr, len + 1);
    (*help)[len] = '\0';

    return BFP_OK;
  }

  // Wait for the process to complete, kill it if we time out
  bfi_Status ProcessWait(Process* process, uint32_t timeout, int32_t* exitCode) {
    m_ctx->Logger().InfoFormat("Waiting for %s seconds for remote process to complete ...", timeout == 0 ? "infinite" : std::to_string(timeout).c_str());
    u32 reason = process->Wait(timeout * 1000);

    if (reason == WAIT_TIMEOUT || reason == WAIT_ABANDONED) {
      m_ctx->Logger().Warn("Process timed out or wait was abandoned. Killing process ...");
      KillProcess(m_ctx.get(), process->GetPid());
    }

    if (exitCode) {
      const u32* ec = process->GetExitCode();
      *exitCode = ec ? *ec : STILL_ACTIVE;
    }
    return BFP_OK;
  }

  // Poll the process and set the error code if it completed
  bfi_Status ProcessPoll(Process* process, int32_t* running, int32_t* exitCode) {
    const u32* ec = process->GetExitCode();
    if (ec) {
      if (running) *running = 0;
      if (exitCode) *exitCode = *ec;
    } else {
      if (running) *running = 1;
      if (exitCode) *exitCode = STILL_ACTIVE;
    }
    return BFP_OK;
  }

  // Kill the process
  bfi_Status ProcessKill(Process* process) {
    KillProcess(m_ctx.get(), process->GetPid());
    return BFP_OK;
  }

  // Get the path of the of the bifrost loader library so we can find it again from the remote process
  std::wstring GetPathOfBifrostLoader() {
    auto handle = m_loader->GetOrLoadModule("bifrost_loader", {L"bifrost_loader.dll"});
    BIFROST_CHECK_WIN_CALL_CTX(m_ctx.get(), ::DisableThreadLibraryCalls(handle) != 0);
    return m_loader->GetModulePath(handle);
  }

  // Create the injector parameters
  InjectorParam MakeInjectorParam(const std::string& customArgs) {
    InjectorParam param;
    param.Pid = ::GetCurrentProcessId();
    param.SharedMemoryName = m_ctx->Memory().GetName();
    param.SharedMemorySize = (u32)m_ctx->Memory().GetSizeInBytes();
    param.CustomArgument = customArgs;

    std::wstring cwd(2 * MAX_PATH, '\0');
    DWORD len = (u32)cwd.size();
    BIFROST_ASSERT_CALL_CTX(m_ctx.get(), (len = ::GetCurrentDirectory(len, (wchar_t*)cwd.data())) != 0);
    param.WorkingDirectory = cwd.substr(0, len);
    m_ctx->Logger().DebugFormat(L"Using working directory: \"%s\"", param.WorkingDirectory.c_str());
    return param;
  }

  // Create/Connect to shared memory
  void CreateOrReuseExistingSharedMemory(const char* sharedMemoryName, u64 sharedMemorySizeInBytes) {
    std::unique_ptr<SharedMemory> memory;

    // Reuse existing memory if it's the same name/size specification
    if (!m_memory || (sharedMemoryName != nullptr && std::string_view(sharedMemoryName) != std::string_view(m_memory->GetName())) ||
        (sharedMemorySizeInBytes != 0 && sharedMemorySizeInBytes != m_memory->GetSizeInBytes())) {
      std::string smName = sharedMemoryName ? sharedMemoryName : UUID(m_ctx.get());
      u64 smSize = (u64)sharedMemorySizeInBytes;
      memory = std::make_unique<SharedMemory>(m_ctx.get(), smName, smSize);
    }

    if (memory) {
      m_memory = std::move(memory);
      m_ctx->SetMemory(m_memory.get());
      SetUpLogConsumer();
    }
  }

  void AttachDebugger(const bfi_InjectorArguments* args, Process* proc, Process::InjectArguments& injectArguments) {
    if (!m_debugger) {
      m_debugger = std::make_unique<Debugger>(m_ctx.get());
      m_debugger->Attach(proc->GetPid(), args->VSSolution);
    }

    // Wait forever in the injection process
    injectArguments.TimeoutInMs = INFINITE;
  }

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
    return BFP_OK;
  }

  // Logging
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
  std::unique_ptr<Debugger> m_debugger;

  std::unique_ptr<BufferedLogger> m_bufferedLogger;
  std::unique_ptr<ForwardLogger> m_forwardLogger;
};

InjectorContext* Get(bfi_Context* ctx) { return (InjectorContext*)ctx->_Internal; }
Process* Get(bfi_Process* process) { return (Process*)process->_Internal; }
Process* Get(const bfi_Process* process) { return (Process*)process->_Internal; }

}  // namespace

#pragma region Version

bfi_Version bfi_GetVersion(void) { return {BIFROST_INJECTOR_VERSION_MAJOR, BIFROST_INJECTOR_VERSION_MINOR, BIFROST_INJECTOR_VERSION_PATCH}; }

const char* bfi_GetVersionString(void) {
  return BIFROST_STRINGIFY(BIFROST_INJECTOR_VERSION_MAJOR) "." BIFROST_STRINGIFY(BIFROST_INJECTOR_VERSION_MINOR) "." BIFROST_STRINGIFY(
      BIFROST_INJECTOR_VERSION_PATCH);
}

#pragma endregion

#pragma region Context

bfi_Context* bfi_ContextInit(void) { return Init<bfi_Context, InjectorContext>(); }

void bfi_ContextFree(bfi_Context* ctx) { Free<bfi_Context, InjectorContext>(ctx); }

const char* bfi_ContextGetLastError(bfi_Context* ctx) { return Get(ctx)->GetLastError(); }

bfi_Status bfi_ContextSetLoggingCallback(bfi_Context* ctx, bfi_LoggingCallback cb) {
  BIFROST_INJECTOR_CATCH_ALL({ return Get(ctx)->SetLogCallback(cb); })
}

#pragma endregion

#pragma region Process

BIFROST_INJECTOR_API bfi_Status bfi_ProcessLaunch(bfi_Context* ctx, const wchar_t* path, const char* arguments, bfi_Process_t** process) {
  BIFROST_INJECTOR_CATCH_ALL({
    *process = (Init<bfi_Process, Process>(Get(ctx)->GetContext(), Process::LaunchArguments{path ? path : L"", arguments ? arguments : ""}));
    return BFP_OK;
  });
}

BIFROST_INJECTOR_API bfi_Status bfi_ProcessFromPid(bfi_Context* ctx, uint32_t pid, bfi_Process_t** process) {
  BIFROST_INJECTOR_CATCH_ALL({
    *process = (Init<bfi_Process, Process>(Get(ctx)->GetContext(), pid));
    return BFP_OK;
  });
}

BIFROST_INJECTOR_API bfi_Status bfi_ProcessFromName(bfi_Context* ctx, const wchar_t* name, bfi_Process_t** process) {
  BIFROST_INJECTOR_CATCH_ALL({
    *process = (Init<bfi_Process, Process>(Get(ctx)->GetContext(), std::wstring(name)));
    return BFP_OK;
  });
}

BIFROST_INJECTOR_API bfi_Status bfi_ProcessFree(bfi_Context* ctx, bfi_Process* process) {
  BIFROST_INJECTOR_CATCH_ALL({
    (Free<bfi_Process, Process>(process));
    return BFP_OK;
  });
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
    KillProcess(Get(ctx)->GetContext(), name, true);
    return BFP_OK;
  });
}

BIFROST_INJECTOR_API bfi_Status bfi_ProcessKillByPid(bfi_Context* ctx, uint32_t pid) {
  BIFROST_INJECTOR_CATCH_ALL({
    KillProcess(Get(ctx)->GetContext(), pid, true);
    return BFP_OK;
  });
}

#pragma endregion

#pragma region Plugin

bfi_Status bfi_PluginLoad(bfi_Context* ctx, const bfi_PluginLoadArguments* args, bfi_Process_t** process, bfi_PluginLoadResult** result) {
  BIFROST_INJECTOR_CATCH_ALL({ return Get(ctx)->PluginLoad(args, process, result); });
}

BIFROST_INJECTOR_API bfi_Status bfi_PluginLoadResultFree(bfi_Context* ctx, bfi_PluginLoadResult* result) {
  BIFROST_INJECTOR_CATCH_ALL({
    if (result) {
      if (result->SharedMemoryName) delete[] result->SharedMemoryName;
      delete result;
    }
    return BFP_OK;
  });
}

BIFROST_INJECTOR_API bfi_Status bfi_PluginUnload(bfi_Context* ctx, const bfi_PluginUnloadArguments* args, const bfi_Process_t* process,
                                                 bfi_PluginUnloadResult** result) {
  BIFROST_INJECTOR_CATCH_ALL({ return Get(ctx)->PluginUnload(args, Get(process), result); });
}

BIFROST_INJECTOR_API bfi_Status bfi_PluginUnloadResultFree(bfi_Context* ctx, bfi_PluginUnloadResult* result) {
  BIFROST_INJECTOR_CATCH_ALL({
    if (result) {
      if (result->Unloaded) delete[] result->Unloaded;
      delete result;
    }
    return BFP_OK;
  });
}

BIFROST_INJECTOR_API bfi_Status bfi_PluginHelp(bfi_Context* ctx, const wchar_t* path, char** help) {
  BIFROST_INJECTOR_CATCH_ALL({ return Get(ctx)->PluginHelp(path, help); });
}

BIFROST_INJECTOR_API bfi_Status bfi_PluginHelpFree(bfi_Context* ctx, char* help) {
  BIFROST_INJECTOR_CATCH_ALL({
    if (help) delete help;
    return BFP_OK;
  });
}

#pragma endregion

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) { return TRUE; }
