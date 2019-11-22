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

namespace bifrost {

std::unique_ptr<TestEnviroment> TestEnviroment::s_instance = nullptr;

std::wstring TestEnviroment::GetInjectorExecutable() const { return GetFile(L"executable", L"test-bifrost-api-injector-executable.exe"); }

std::wstring TestEnviroment::GetInjectorPlugin() const { return GetFile(L"plugin", L"test-bifrost-api-injector-plugin.dll"); }

std::wstring TestEnviroment::GetHookExecutable() const { return GetFile(L"executable", L"test-bifrost-api-hook-executable.exe"); }

std::wstring TestEnviroment::GetHookDll() const { return GetFile(L"plugin", L"test-bifrost-api-hook-dll.dll"); }

std::wstring TestEnviroment::GetHookPlugin() const { return GetFile(L"dll", L"test-bifrost-api-hook-plugin.dll"); }

std::wstring TestEnviroment::GetHookPlugin2() const { return GetFile(L"dll", L"test-bifrost-api-hook-plugin-2.dll"); }

std::string TestEnviroment::GetTmpFile(Context* ctx) const { return (std::filesystem::temp_directory_path() / UUID(ctx)).string(); }

void LogCallback(u32 level, const char* module, const char* msg) { TestEnviroment::Get().GetLogger()->Sink((ILogger::LogLevel)level, module, msg); }

void TestInjectorBase::SetUp() {
  m_ctx = std::shared_ptr<bfi_Context>(bfi_ContextInit(), [](bfi_Context* c) { bfi_ContextFree(c); });
  if (m_ctx == nullptr) throw std::runtime_error("Failed to initialize bifrost injector context");
  BIFROST_EXPECT_OK(bfi_ContextSetLoggingCallback(GetContext(), LogCallback));

  m_bfCtx = std::make_unique<Context>();
  m_bfCtx->SetLogger(TestEnviroment::Get().GetLogger());
}

void TestInjectorBase::TearDown() { m_ctx.reset(); }

bfi_Context* TestInjectorBase::GetContext() const { return m_ctx.get(); }

bifrost::Context* TestInjectorBase::GetBifrostContext() const { return m_bfCtx.get(); }

std::string TestInjectorBase::GetTmpFile() { return TestEnviroment::Get().GetTmpFile(m_bfCtx.get()); }

std::string TestInjectorBase::GetContent(std::string file) {
  if (!std::filesystem::exists(file)) throw std::runtime_error("File does not exist: \"" + file + "\"");
  std::ifstream ifs(file);
  return {std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
}

std::shared_ptr<bfi_InjectorArguments> TestInjectorBase::MakeInjectorArguments(std::string sharedMemoryName /*= ""*/) {
  auto args = std::make_shared<bfi_InjectorArguments>();

  args->TimeoutInS = BIFROST_INJECTOR_DEFAULT_InjectorArguments_TimeoutInS;
  args->SharedMemoryName = sharedMemoryName.empty() ? nullptr : m_mem.CopyString(sharedMemoryName);
  args->SharedMemorySizeInBytes = BIFROST_INJECTOR_DEFAULT_InjectorArguments_SharedMemorySizeInBytes;
  args->Debugger = ::IsDebuggerPresent();
  args->VSSolution = L"bifrost";
  return args;
}

std::shared_ptr<bfi_ExecutableArguments> TestInjectorBase::MakeExecutableArgumentsForLaunchImpl(std::wstring executable, std::string arguments) {
  auto args = std::make_shared<bfi_ExecutableArguments>();
  args->Mode = BFI_LAUNCH;
  args->ExecutablePath = m_mem.CopyString(executable);
  args->ExecutableArguments = m_mem.CopyString(arguments);
  return args;
}

std::shared_ptr<bfi_ExecutableArguments> TestInjectorBase::MakeExecutableArgumentsForConnect(u32 pid) {
  auto args = std::make_shared<bfi_ExecutableArguments>();
  args->Mode = BFI_CONNECT_VIA_PID;
  args->Pid = pid;
  return args;
}

std::vector<bfi_PluginLoadDesc> TestInjectorBase::MakePluginLoadDescImpl(std::string name, std::wstring path, std::string arguments, bool forceLoad) {
  std::vector<bfi_PluginLoadDesc> plugins;
  plugins.emplace_back(
      bfi_PluginLoadDesc{m_mem.CopyString(name.c_str()), m_mem.CopyString(path), arguments.empty() ? "" : m_mem.CopyString(arguments), forceLoad});
  return plugins;
}

std::vector<bfi_PluginUnloadDesc> TestInjectorBase::MakePluginUnloadDescImpl(std::string name) {
  std::vector<bfi_PluginUnloadDesc> plugins;
  plugins.emplace_back(bfi_PluginUnloadDesc{m_mem.CopyString(name.c_str())});
  return plugins;
}

std::shared_ptr<bfi_PluginLoadArguments> TestInjectorBase::MakePluginLoadArguments(const std::shared_ptr<bfi_ExecutableArguments>& executableArguments,
                                                                                   const std::shared_ptr<bfi_InjectorArguments>& injectorArguments,
                                                                                   std::vector<bfi_PluginLoadDesc>& pluginLoadDesc) {
  auto args = std::make_shared<bfi_PluginLoadArguments>();
  args->Executable = executableArguments.get();
  args->InjectorArguments = injectorArguments.get();
  args->Plugins = pluginLoadDesc.data();
  args->NumPlugins = (u32)pluginLoadDesc.size();
  return args;
}

std::shared_ptr<bfi_Process> TestInjectorBase::GetProcessFromPid(u32 pid) {
  bfi_Process* bfiProcess = nullptr;
  BIFROST_EXPECT_OK(bfi_ProcessFromPid(GetContext(), pid, &bfiProcess));
  return std::shared_ptr<bfi_Process>(bfiProcess, [&](bfi_Process* p) { bfi_ProcessFree(GetContext(), p); });
}

std::shared_ptr<bfi_PluginUnloadArguments> TestInjectorBase::MakePluginUnloadArguments(const std::shared_ptr<bfi_InjectorArguments>& injectorArguments,
                                                                                       std::vector<bfi_PluginUnloadDesc>& pluginUnloadDesc) {
  auto args = std::make_shared<bfi_PluginUnloadArguments>();
  args->InjectorArguments = injectorArguments.get();
  args->Plugins = pluginUnloadDesc.data();
  args->NumPlugins = (u32)pluginUnloadDesc.size();
  return args;
}

bifrost::TestInjectorBase::LoadResult TestInjectorBase::Load(std::shared_ptr<bfi_PluginLoadArguments> loadArguments) {
  bfi_Process* bfiProcess = nullptr;
  bfi_PluginLoadResult* bfiPluginLoadResult = nullptr;

  BIFROST_EXPECT_OK(bfi_PluginLoad(GetContext(), loadArguments.get(), &bfiProcess, &bfiPluginLoadResult));

  return LoadResult{std::shared_ptr<bfi_Process>(bfiProcess, [&](bfi_Process* p) { bfi_ProcessFree(GetContext(), p); }),
                    std::shared_ptr<bfi_PluginLoadResult>(bfiPluginLoadResult, [&](bfi_PluginLoadResult* p) { bfi_PluginLoadResultFree(GetContext(), p); })};
}

bifrost::TestInjectorBase::UnloadResult TestInjectorBase::Unload(std::shared_ptr<bfi_PluginUnloadArguments> unloadArguments,
                                                                 std::shared_ptr<bfi_Process> process) {
  bfi_Process* bfiProcess = nullptr;
  bfi_PluginUnloadResult* bfiPluginUnloadResult = nullptr;

  BIFROST_EXPECT_OK(bfi_PluginUnload(GetContext(), unloadArguments.get(), process.get(), &bfiPluginUnloadResult));

  return UnloadResult{
      std::shared_ptr<bfi_PluginUnloadResult>(bfiPluginUnloadResult, [&](bfi_PluginUnloadResult* p) { bfi_PluginUnloadResultFree(GetContext(), p); })};
}

bifrost::i32 TestInjectorBase::Wait(const std::shared_ptr<bfi_Process>& process, i32 timeout /*= 10*/) {
  i32 exitCode = -1;
  BIFROST_EXPECT_OK(bfi_ProcessWait(GetContext(), process.get(), timeout, &exitCode));
  return exitCode;
}

std::shared_ptr<char> TestInjectorBase::HelpImpl(std::wstring plugin) {
  char* help = nullptr;
  BIFROST_EXPECT_OK(bfi_PluginHelp(GetContext(), plugin.c_str(), &help));
  return std::shared_ptr<char>(help, [&](char* p) { bfi_PluginHelpFree(GetContext(), p); });
}

}  // namespace bifrost
