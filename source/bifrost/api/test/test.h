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

#pragma once

#include "bifrost/core/test/test_shared.h"
#include "bifrost/api/injector.h"

namespace bifrost {

/// Global test environment
class TestEnviroment final : public ::testing::Environment, public BaseTestEnviroment {
 public:
  TestEnviroment() { m_logger = std::make_unique<TestLogger>(); }

  std::wstring GetInjectorExecutable() const;
  std::wstring GetInjectorPlugin() const;
  std::wstring GetHookExecutable() const;
  std::wstring GetHookDll() const;
  std::wstring GetHookPlugin() const;
  std::wstring GetHookPlugin2() const;
  std::wstring GetHookPlugin3() const;

  /// Create a temporary file
  std::string GetTmpFile(Context* ctx) const;

  /// Get the test logger
  ILogger* GetLogger() { return m_logger.get(); }

  static TestEnviroment& Get() { return *s_instance; }

  virtual void SetUp() override { s_instance = std::make_unique<TestEnviroment>(); }
  virtual void TearDown() override { s_instance.reset(); }

 private:
  static std::unique_ptr<TestEnviroment> s_instance;
  std::unique_ptr<ILogger> m_logger = nullptr;
};

namespace internal {

struct MemoryPool {
  std::vector<std::unique_ptr<char>> StrMem;
  std::vector<std::unique_ptr<wchar_t>> WstrMem;

  const char* CopyString(const std::string& str) { return StrMem.emplace_back(StringCopy(str)).get(); }
  const wchar_t* CopyString(const std::wstring& str) { return WstrMem.emplace_back(StringCopy(str)).get(); };
};

}  // namespace internal

#define BIFROST_EXPECT_OK(expr) EXPECT_TRUE((expr) == BFP_OK) << bfi_ContextGetLastError(GetContext())

/// Base class of injector based tests
class TestInjectorBase : public ::testing::Test {
 public:
  virtual void SetUp() override;
  virtual void TearDown() override;

  bfi_Context* GetContext() const;
  Context* GetBifrostContext() const;

  /// Create a temporary file and get it's path
  std::string GetTmpFile();

  /// Get the content of `file`
  std::string GetContent(std::string file);

  /// Create bfi_PluginLoadArguments
  std::shared_ptr<bfi_InjectorArguments> MakeInjectorArguments(std::string sharedMemoryName = "");

  /// Create bfi_ExecutableArguments for launching the test executable
  std::shared_ptr<bfi_ExecutableArguments> MakeExecutableArgumentsForLaunchImpl(std::wstring executable, std::string arguments);

  /// Create bfi_ExecutableArguments for connecting to the test executable
  std::shared_ptr<bfi_ExecutableArguments> MakeExecutableArgumentsForConnect(u32 pid);

  /// Create bfi_PluginLoadDesc vector for the test plugin
  std::vector<bfi_PluginLoadDesc> MakePluginLoadDescVecImpl(std::string name, std::wstring path, std::string arguments = "", bool forceLoad = false);
  bfi_PluginLoadDesc MakePluginLoadDescImpl(std::string name, std::wstring path, std::string arguments = "", bool forceLoad = false);

  /// Create bfi_PluginUnloadDesc for the test plugin
  std::vector<bfi_PluginUnloadDesc> MakePluginUnloadDescImpl(std::string name);

  /// Create bfi_PluginLoadArguments
  std::shared_ptr<bfi_PluginLoadArguments> MakePluginLoadArguments(const std::shared_ptr<bfi_ExecutableArguments>& executableArguments,
                                                                   const std::shared_ptr<bfi_InjectorArguments>& injectorArguments,
                                                                   std::vector<bfi_PluginLoadDesc>& pluginLoadDesc);

  /// Create bfi_Process from a pid
  std::shared_ptr<bfi_Process> GetProcessFromPid(u32 pid);

  /// Create bfi_PluginLoadArguments
  std::shared_ptr<bfi_PluginUnloadArguments> MakePluginUnloadArguments(const std::shared_ptr<bfi_InjectorArguments>& injectorArguments,
                                                                       std::vector<bfi_PluginUnloadDesc>& pluginUnloadDesc);

  /// Load plugin
  struct LoadResult {
    std::shared_ptr<bfi_Process> Process;
    std::shared_ptr<bfi_PluginLoadResult> Result;
  };
  LoadResult Load(std::shared_ptr<bfi_PluginLoadArguments> loadArguments);

  /// Unload plugin
  struct UnloadResult {
    std::shared_ptr<bfi_PluginUnloadResult> Result;
  };
  UnloadResult Unload(std::shared_ptr<bfi_PluginUnloadArguments> unloadArguments, std::shared_ptr<bfi_Process> process);

  /// Wait for `timeout` seconds for process to complete
  i32 Wait(const std::shared_ptr<bfi_Process>& process, i32 timeout = 60);

  /// Get plugin help
  std::shared_ptr<char> HelpImpl(std::wstring plugin);

 private:
  internal::MemoryPool m_mem;
  std::shared_ptr<bfi_Context> m_ctx;
  std::shared_ptr<Context> m_bfCtx;
};

/// Logging callback
extern void LogCallback(u32 level, const char* module, const char* msg);

}  // namespace bifrost