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

#include "bifrost/api/test/data/plugin_def.h"

namespace {

using namespace bifrost;

class TestHook : public TestInjectorBase {
 public:
  std::shared_ptr<bfi_ExecutableArguments> MakeExecutableArgumentsForLaunch(std::string file, i32 arg1 = 1, i32 arg2 = 2, i32 sleep = 0) {
    return MakeExecutableArgumentsForLaunchImpl(TestEnviroment::Get().GetHookExecutable(),
                                                file + " " + std::to_string(arg1) + " " + std::to_string(arg2) + " " + std::to_string(sleep));
  }

  std::vector<bfi_PluginLoadDesc> MakePluginLoadDesc(std::string file, Function function = Function::none, i32 plugin = 1) {
    return MakePluginLoadDescImpl("InjectorTestPlugin", plugin == 1 ? TestEnviroment::Get().GetHookPlugin1() : TestEnviroment::Get().GetHookPlugin2(),
                                  file + ";" + std::to_string((int)function), false);
  }
};

TEST_F(TestHook, NoHooks) {
  auto tmpFile = GetTmpFile();

  auto launchArgs = MakeExecutableArgumentsForLaunch(tmpFile);
  auto injectorArgs = MakeInjectorArguments();
  auto pluginLoadDesc = MakePluginLoadDesc(tmpFile);

  auto loadArgs = MakePluginLoadArguments(launchArgs, injectorArgs, pluginLoadDesc);
  auto loadResult = Load(loadArgs);
  ASSERT_EQ(Wait(loadResult.Process), 0);

  ASSERT_STREQ(GetContent(tmpFile).c_str(), "SetUp1:Result=3:TearDown1:") << "File: " << tmpFile;
}

TEST_F(TestHook, Original) {
  auto tmpFile = GetTmpFile();

  auto launchArgs = MakeExecutableArgumentsForLaunch(tmpFile);
  auto injectorArgs = MakeInjectorArguments();
  auto pluginLoadDesc = MakePluginLoadDesc(tmpFile, Function::my_bifrost_add__original);

  auto loadArgs = MakePluginLoadArguments(launchArgs, injectorArgs, pluginLoadDesc);
  auto loadResult = Load(loadArgs);
  ASSERT_EQ(Wait(loadResult.Process), 0);

  ASSERT_STREQ(GetContent(tmpFile).c_str(), "SetUp1:Result=3:TearDown1:") << "File: " << tmpFile;
}

}  // namespace