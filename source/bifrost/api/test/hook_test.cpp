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

//class TestHook : public TestInjectorBase {
// public:
//  std::shared_ptr<bfi_ExecutableArguments> MakeExecutableArgumentsForLaunch(u32 sleepForMs = 1500, i32 returnCode = 0) {
//    return MakeExecutableArgumentsForLaunchImpl(TestEnviroment::Get().GetInjectorExecutable(), std::to_string(returnCode) + " " + std::to_string(sleepForMs));
//  }
//
//  std::vector<bfi_PluginLoadDesc> MakePluginLoadDesc(std::string arguments = "", bool forceLoad = false) {
//    return MakePluginLoadDescImpl("InjectorTestPlugin", TestEnviroment::Get().GetInjectorPlugin(), arguments, forceLoad);
//  }
//
//	  std::vector<bfi_PluginLoadDesc> MakePluginLoadDesc(std::string arguments = "", bool forceLoad = false) {
//    return MakePluginLoadDescImpl("InjectorTestPlugin", TestEnviroment::Get().GetInjectorPlugin(), arguments, forceLoad);
//  }
//
//  std::vector<bfi_PluginUnloadDesc> MakePluginUnloadDesc() { return MakePluginUnloadDescImpl("InjectorTestPlugin"); }
//
//  std::shared_ptr<char> Help() { return HelpImpl(TestEnviroment::Get().GetInjectorPlugin()); }
//};
//
//TEST_F(TestHook, LoadAndUnload) {
//  auto tmpFile = GetTmpFile();
//
//  auto launchArgs = MakeExecutableArgumentsForLaunch();
//  auto injectorArgs = MakeInjectorArguments();
//  auto pluginLoadDesc = MakePluginLoadDesc(tmpFile);
//
//  // Load
//  auto loadArgs = MakePluginLoadArguments(launchArgs, injectorArgs, pluginLoadDesc);
//  auto loadResult = Load(loadArgs);
//  ASSERT_EQ(Wait(loadResult.Process), 0);
//  ASSERT_STREQ(GetContent(tmpFile).c_str(), "SetUp:") << "File: " << tmpFile;
//}

}  // namespace