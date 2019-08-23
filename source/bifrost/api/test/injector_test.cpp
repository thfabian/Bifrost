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
  }
  virtual void TearDown() override { m_ctx.reset(); }

  bfi_Context* GetContext() const { return m_ctx.get(); }

  /// Create bfi_PluginLoadArguments
  std::shared_ptr<bfi_InjectorArguments_t> MakeInjectorArguments(std::string sharedMemoryName = "",
                                                                 u32 sharedMemorySize = BIFROST_INJECTOR_DEFAULT_InjectorArguments_SharedMemorySizeInBytes) {
    auto args = std::make_shared<bfi_InjectorArguments_t>();
    args->SharedMemoryName = sharedMemoryName.empty() ? nullptr : m_mem.CopyString(sharedMemoryName);
    args->TimeoutInS = BIFROST_INJECTOR_DEFAULT_InjectorArguments_TimeoutInS;
    args->SharedMemorySizeInBytes = sharedMemorySize;
    args->Debugger = ::IsDebuggerPresent();
    args->VSSolution = L"bifrost";
    return args;
  }

  /// Create bfi_ExecutableArguments for launching the test executable
  std::shared_ptr<bfi_ExecutableArguments> MakeExecutableArgumentsForLaunch(u32 sleepForMs = 1000, i32 returnCode = 0) {
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

  /// Create bfi_ExecutableArguments for connecting to the test executable
  std::vector<bfi_PluginLoadDesc> MakePluginLoadDesc(std::string arguments = "", bool forceLoad = false) {
    std::vector<bfi_PluginLoadDesc> plugins;
    plugins.emplace_back(bfi_PluginLoadDesc{"InjectorTest", m_mem.CopyString(TestEnviroment::Get().GetInjectorPlugin()),
                                            arguments.empty() ? "" : m_mem.CopyString(arguments), forceLoad});
    return plugins;
  }

  /// Create bfi_PluginLoadArguments
  std::shared_ptr<bfi_PluginLoadArguments> MakePluginLoadArguments(std::shared_ptr<bfi_ExecutableArguments> executableArguments,
                                                                   std::shared_ptr<bfi_InjectorArguments> injectorArguments,
                                                                   std::vector<bfi_PluginLoadDesc>& pluginLoadDesc) {
    auto args = std::make_shared<bfi_PluginLoadArguments>();
    args->Executable = executableArguments.get();
    args->InjectorArguments = injectorArguments.get();
    args->Plugins = pluginLoadDesc.data();
    args->NumPlugins = (u32)pluginLoadDesc.size();
    return args;
  }

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

 private:
  MemoryPool m_mem;
  std::shared_ptr<bfi_Context> m_ctx;
};

TEST_F(TestInjector, Load) {
  auto exeArgs = MakeExecutableArgumentsForLaunch();
  auto injectorArgs = MakeInjectorArguments();
  auto pluginLoadDesc = MakePluginLoadDesc();

  auto loadArgs = MakePluginLoadArguments(exeArgs, injectorArgs, pluginLoadDesc);
  auto loadResult = Load(loadArgs);
}

TEST_F(TestInjector, Unload) {}

}  // namespace