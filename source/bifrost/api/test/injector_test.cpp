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

#include "bifrost/api/test/test.h"

#include "bifrost/api/injector.h"

#define BIFROST_EXPECT_OK(expr) EXPECT_TRUE((expr) == BFP_OK) << bfi_ContextGetLastError(GetContext())

namespace {

using namespace bifrost;

/// Keep track of string memory
struct MemoryPool {
  std::vector<std::unique_ptr<char>> StrMem;
  std::vector<std::unique_ptr<wchar_t>> WstrMem;

  const char* CopyString(const std::string& str) { return StrMem.emplace_back(StringCopy(str)).get(); }
  const wchar_t* CopyString(const std::wstring& str) { return WstrMem.emplace_back(StringCopy(str)).get(); };
};

class TestInjector : public ::testing::Test {
 public:
  virtual void SetUp() override {
    m_ctx = std::shared_ptr<bfi_Context>(bfi_ContextInit(), [](bfi_Context* c) { bfi_ContextFree(c); });
    if (m_ctx == nullptr) throw std::runtime_error("Failed to initialize bifrost injector context");
    BIFROST_EXPECT_OK(bfi_ContextSetLoggingCallback(GetContext(), LogCallback));

    m_bfCtx = std::make_unique<Context>();
    m_bfCtx->SetLogger(TestEnviroment::Get().GetLogger());
  }
  virtual void TearDown() override { m_ctx.reset(); }

  bfi_Context* GetContext() const { return m_ctx.get(); }

  /// Create bfi_PluginLoadArguments
  std::shared_ptr<bfi_InjectorArguments> MakeInjectorArguments(std::string sharedMemoryName = "") {
    auto args = std::make_shared<bfi_InjectorArguments>();

    args->TimeoutInS = BIFROST_INJECTOR_DEFAULT_InjectorArguments_TimeoutInS;
    args->SharedMemoryName = sharedMemoryName.empty() ? nullptr : m_mem.CopyString(sharedMemoryName);
    args->SharedMemorySizeInBytes = BIFROST_INJECTOR_DEFAULT_InjectorArguments_SharedMemorySizeInBytes;
    args->Debugger = ::IsDebuggerPresent();
    args->VSSolution = L"bifrost";
    return args;
  }

  /// Create bfi_ExecutableArguments for launching the test executable
  std::shared_ptr<bfi_ExecutableArguments> MakeExecutableArgumentsForLaunch(u32 sleepForMs = 1500, i32 returnCode = 0) {
    auto args = std::make_shared<bfi_ExecutableArguments>();
    args->Mode = BFI_LAUNCH;
    args->ExecutablePath = m_mem.CopyString(TestEnviroment::Get().GetInjectorExecutable());
    args->ExecutableArguments = m_mem.CopyString(std::to_string(returnCode) + " " + std::to_string(sleepForMs));
    return args;
  }

  /// Create bfi_ExecutableArguments for connecting to the test executable
  std::shared_ptr<bfi_ExecutableArguments> MakeExecutableArgumentsForConnect(u32 pid) {
    auto args = std::make_shared<bfi_ExecutableArguments>();
    args->Mode = BFI_CONNECT_VIA_PID;
    args->Pid = pid;
    return args;
  }

  /// Create bfi_PluginLoadDesc for the test plugin
  std::vector<bfi_PluginLoadDesc> MakePluginLoadDesc(std::string arguments = "", bool forceLoad = false) {
    std::vector<bfi_PluginLoadDesc> plugins;
    plugins.emplace_back(bfi_PluginLoadDesc{"InjectorTest", m_mem.CopyString(TestEnviroment::Get().GetInjectorPlugin()),
                                            arguments.empty() ? "" : m_mem.CopyString(arguments), forceLoad});
    return plugins;
  }

  /// Create bfi_PluginUnloadDesc for the test plugin
  std::vector<bfi_PluginUnloadDesc> MakePluginUnloadDesc() {
    std::vector<bfi_PluginUnloadDesc> plugins;
    plugins.emplace_back(bfi_PluginUnloadDesc{"InjectorTest"});
    return plugins;
  }

  /// Create bfi_PluginLoadArguments
  std::shared_ptr<bfi_PluginLoadArguments> MakePluginLoadArguments(const std::shared_ptr<bfi_ExecutableArguments>& executableArguments,
                                                                   const std::shared_ptr<bfi_InjectorArguments>& injectorArguments,
                                                                   std::vector<bfi_PluginLoadDesc>& pluginLoadDesc) {
    auto args = std::make_shared<bfi_PluginLoadArguments>();
    args->Executable = executableArguments.get();
    args->InjectorArguments = injectorArguments.get();
    args->Plugins = pluginLoadDesc.data();
    args->NumPlugins = (u32)pluginLoadDesc.size();
    return args;
  }

  std::shared_ptr<bfi_Process> GetProcessFromPid(u32 pid) {
    bfi_Process* bfiProcess = nullptr;
    BIFROST_EXPECT_OK(bfi_ProcessFromPid(GetContext(), pid, &bfiProcess));
    return std::shared_ptr<bfi_Process>(bfiProcess, [&](bfi_Process* p) { bfi_ProcessFree(GetContext(), p); });
  }

  /// Create bfi_PluginLoadArguments
  std::shared_ptr<bfi_PluginUnloadArguments> MakePluginUnloadArguments(const std::shared_ptr<bfi_InjectorArguments>& injectorArguments,
                                                                       std::vector<bfi_PluginUnloadDesc>& pluginUnloadDesc) {
    auto args = std::make_shared<bfi_PluginUnloadArguments>();
    args->InjectorArguments = injectorArguments.get();
    args->Plugins = pluginUnloadDesc.data();
    args->NumPlugins = (u32)pluginUnloadDesc.size();
    return args;
  }

  /// Load plugin
  struct LoadResult {
    std::shared_ptr<bfi_Process> Process;
    std::shared_ptr<bfi_PluginLoadResult> Result;
  };
  LoadResult Load(std::shared_ptr<bfi_PluginLoadArguments> loadArguments) {
    bfi_Process* bfiProcess = nullptr;
    bfi_PluginLoadResult* bfiPluginLoadResult = nullptr;

    BIFROST_EXPECT_OK(bfi_PluginLoad(GetContext(), loadArguments.get(), &bfiProcess, &bfiPluginLoadResult));

    return LoadResult{std::shared_ptr<bfi_Process>(bfiProcess, [&](bfi_Process* p) { bfi_ProcessFree(GetContext(), p); }),
                      std::shared_ptr<bfi_PluginLoadResult>(bfiPluginLoadResult, [&](bfi_PluginLoadResult* p) { bfi_PluginLoadResultFree(GetContext(), p); })};
  }

  /// Unload plugin
  struct UnloadResult {
    std::shared_ptr<bfi_PluginUnloadResult> Result;
  };
  UnloadResult Unload(std::shared_ptr<bfi_PluginUnloadArguments> unloadArguments, std::shared_ptr<bfi_Process> process) {
    bfi_Process* bfiProcess = nullptr;
    bfi_PluginUnloadResult* bfiPluginUnloadResult = nullptr;

    BIFROST_EXPECT_OK(bfi_PluginUnload(GetContext(), unloadArguments.get(), process.get(), &bfiPluginUnloadResult));

    return UnloadResult{
        std::shared_ptr<bfi_PluginUnloadResult>(bfiPluginUnloadResult, [&](bfi_PluginUnloadResult* p) { bfi_PluginUnloadResultFree(GetContext(), p); })};
  }

  /// Wait for `timeout` seconds for process to complete
  i32 Wait(const std::shared_ptr<bfi_Process>& process, i32 timeout = 10) {
    i32 exitCode = -1;
    BIFROST_EXPECT_OK(bfi_ProcessWait(GetContext(), process.get(), timeout, &exitCode));
    return exitCode;
  }

 private:
  MemoryPool m_mem;
  std::shared_ptr<bfi_Context> m_ctx;
  std::shared_ptr<Context> m_bfCtx;
};

TEST_F(TestInjector, LoadAndWait) {
  auto launchArgs = MakeExecutableArgumentsForLaunch();
  auto injectorArgs = MakeInjectorArguments();
  auto pluginLoadDesc = MakePluginLoadDesc();

  // Load & Wait
  auto loadArgs = MakePluginLoadArguments(launchArgs, injectorArgs, pluginLoadDesc);
  ASSERT_EQ(Wait(Load(loadArgs).Process), 0);
}

TEST_F(TestInjector, LoadAndUnload) {
  auto launchArgs = MakeExecutableArgumentsForLaunch();
  auto injectorArgs = MakeInjectorArguments();
  auto pluginLoadDesc = MakePluginLoadDesc();

  // Load
  auto loadArgs = MakePluginLoadArguments(launchArgs, injectorArgs, pluginLoadDesc);
  auto loadResult = Load(loadArgs);

  // Unload
  auto process = GetProcessFromPid(loadResult.Result->RemoteProcessPid);
  auto pluginUnloadDesc = MakePluginUnloadDesc();
  auto unloadArgs = MakePluginUnloadArguments(injectorArgs, pluginUnloadDesc);
  auto unloadResult = Unload(unloadArgs, process);
  ASSERT_TRUE(unloadResult.Result->Unloaded[0]);

  // Wait
  ASSERT_EQ(Wait(loadResult.Process), 0);
}

}  // namespace