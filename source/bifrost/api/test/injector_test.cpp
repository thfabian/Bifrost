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

namespace {

using namespace bifrost;

class TestInjector : public TestInjectorBase {
 public:
  std::shared_ptr<bfi_ExecutableArguments> MakeExecutableArgumentsForLaunch(u32 sleepForMs = 1500, i32 returnCode = 0) {
    return MakeExecutableArgumentsForLaunchImpl(TestEnviroment::Get().GetInjectorExecutable(), std::to_string(returnCode) + " " + std::to_string(sleepForMs));
  }

  std::vector<bfi_PluginLoadDesc> MakePluginLoadDesc(std::string arguments = "", bool forceLoad = false) {
    return MakePluginLoadDescVecImpl("InjectorTestPlugin", TestEnviroment::Get().GetInjectorPlugin(), arguments, forceLoad);
  }

  std::vector<bfi_PluginUnloadDesc> MakePluginUnloadDesc() { return MakePluginUnloadDescImpl("InjectorTestPlugin"); }

  std::shared_ptr<char> Help() { return HelpImpl(TestEnviroment::Get().GetInjectorPlugin()); }
};

TEST_F(TestInjector, LoadAndUnload) {
  auto tmpFile = GetTmpFile();

  auto launchArgs = MakeExecutableArgumentsForLaunch();
  auto injectorArgs = MakeInjectorArguments();
  auto pluginLoadDesc = MakePluginLoadDesc(tmpFile);

  // Load
  auto loadArgs = MakePluginLoadArguments(launchArgs, injectorArgs, pluginLoadDesc);
  auto loadResult = Load(loadArgs);
  ASSERT_STREQ(GetContent(tmpFile).c_str(), "SetUp:") << "File: " << tmpFile;

  // Unload
  auto process = GetProcessFromPid(loadResult.Result->RemoteProcessPid);
  auto pluginUnloadDesc = MakePluginUnloadDesc();
  auto unloadArgs = MakePluginUnloadArguments(injectorArgs, pluginUnloadDesc);
  auto unloadResult = Unload(unloadArgs, process);
  ASSERT_TRUE(unloadResult.Result->Unloaded[0]);
  ASSERT_STREQ(GetContent(tmpFile).c_str(), "SetUp:TearDown:") << "File: " << tmpFile;

  // Wait
  ASSERT_EQ(Wait(loadResult.Process), 0);
}

TEST_F(TestInjector, LoadLoad) {
  auto tmpFile = GetTmpFile();

  auto injectorArgs = MakeInjectorArguments();
  auto pluginLoadDesc = MakePluginLoadDesc(tmpFile);

  // Load
  auto launchArgs = MakeExecutableArgumentsForLaunch();
  auto loadArgs1 = MakePluginLoadArguments(launchArgs, injectorArgs, pluginLoadDesc);
  auto loadResult1 = Load(loadArgs1);
  ASSERT_STREQ(GetContent(tmpFile).c_str(), "SetUp:") << "File: " << tmpFile;

  // Load
  auto connectArgs = MakeExecutableArgumentsForConnect(loadResult1.Result->RemoteProcessPid);
  auto loadArgs2 = MakePluginLoadArguments(connectArgs, injectorArgs, pluginLoadDesc);
  auto loadResult2 = Load(loadArgs2);
  ASSERT_STREQ(GetContent(tmpFile).c_str(), "SetUp:") << "File: " << tmpFile;  // Skip load as it has already been loaded

  // Wait
  ASSERT_EQ(Wait(loadResult2.Process), 0);
  ASSERT_STREQ(GetContent(tmpFile).c_str(), "SetUp:TearDown:") << "File: " << tmpFile;
}

TEST_F(TestInjector, ForceLoadLoad) {
  auto tmpFile = GetTmpFile();

  auto injectorArgs = MakeInjectorArguments();
  auto pluginLoadDesc = MakePluginLoadDesc(tmpFile);
  pluginLoadDesc[0].ForceLoad = true;

  // Load
  auto launchArgs = MakeExecutableArgumentsForLaunch();
  auto loadArgs1 = MakePluginLoadArguments(launchArgs, injectorArgs, pluginLoadDesc);
  auto loadResult1 = Load(loadArgs1);
  ASSERT_STREQ(GetContent(tmpFile).c_str(), "SetUp:") << "File: " << tmpFile;

  // Load
  auto connectArgs = MakeExecutableArgumentsForConnect(loadResult1.Result->RemoteProcessPid);
  auto loadArgs2 = MakePluginLoadArguments(connectArgs, injectorArgs, pluginLoadDesc);
  auto loadResult2 = Load(loadArgs2);
  ASSERT_STREQ(GetContent(tmpFile).c_str(), "SetUp:TearDown:SetUp:") << "File: " << tmpFile;  // Plugin is already loaded, unload it and reload it

  // Wait
  ASSERT_EQ(Wait(loadResult2.Process), 0);
  ASSERT_STREQ(GetContent(tmpFile).c_str(), "SetUp:TearDown:SetUp:TearDown:") << "File: " << tmpFile;
}

TEST_F(TestInjector, Help) {
  // Help
  auto helpStr = Help();
  ASSERT_STREQ(helpStr.get(), "Help");
}

}  // namespace