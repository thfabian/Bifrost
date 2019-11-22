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

#include "bifrost/api/test/data/hook_plugin.h"

namespace {

using namespace bifrost;

class TestHook : public TestInjectorBase {
 public:
  std::shared_ptr<bfi_ExecutableArguments> MakeExecutableArgumentsForLaunch(std::string file, i32 arg1 = 1, i32 arg2 = 2, i32 sleep = 0) {
    return MakeExecutableArgumentsForLaunchImpl(TestEnviroment::Get().GetHookExecutable(),
                                                file + " " + std::to_string(arg1) + " " + std::to_string(arg2) + " " + std::to_string(sleep));
  }

  std::vector<bfi_PluginLoadDesc> MakePluginLoadDesc(std::string file, Mode mode = Mode::none, i32 plugin = 1) {
    return MakePluginLoadDescImpl("HookTestPlugin", plugin == 1 ? TestEnviroment::Get().GetHookPlugin() : TestEnviroment::Get().GetHookPlugin2(),
                                  file + ";" + std::to_string((int)mode), false);
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

#pragma region CFunction Single

// The tests work the following:
// 1) We pass the mode <id> we want to call via plugin argument. The plugin argument has the form "<tmp-file>;<id>"
// 2) The plugin will hook the mode we specified in <id>.
// 3) The executable will call bifrost_add and the hook should be called.
//
// The executable and the plugin write to the tmp file so we can check the results.

//
//  APP -> bifrost_add__original_1 -> ORIGINAL
//
TEST_F(TestHook, CFunction_Single_Original1) {
  auto tmpFile = GetTmpFile();

  auto launchArgs = MakeExecutableArgumentsForLaunch(tmpFile);
  auto injectorArgs = MakeInjectorArguments();
  auto pluginLoadDesc = MakePluginLoadDesc(tmpFile, Mode::CFunction_Single_Original1);

  auto loadArgs = MakePluginLoadArguments(launchArgs, injectorArgs, pluginLoadDesc);
  auto loadResult = Load(loadArgs);
  ASSERT_EQ(Wait(loadResult.Process), 0);

  ASSERT_STREQ(GetContent(tmpFile).c_str(), "SetUp1:Result=3:TearDown1:") << "File: " << tmpFile;
}

//
//  APP -> bifrost_add__original_2 -> ORIGINAL
//
TEST_F(TestHook, CFunction_Single_Original2) {
  auto tmpFile = GetTmpFile();

  auto launchArgs = MakeExecutableArgumentsForLaunch(tmpFile);
  auto injectorArgs = MakeInjectorArguments();
  auto pluginLoadDesc = MakePluginLoadDesc(tmpFile, Mode::CFunction_Single_Original2);

  auto loadArgs = MakePluginLoadArguments(launchArgs, injectorArgs, pluginLoadDesc);
  auto loadResult = Load(loadArgs);
  ASSERT_EQ(Wait(loadResult.Process), 0);

  ASSERT_STREQ(GetContent(tmpFile).c_str(), "SetUp1:Result=3:TearDown1:") << "File: " << tmpFile;
}

//
//  APP -> bifrost_add__original_3 -> ORIGINAL
//
TEST_F(TestHook, CFunction_Single_Original3) {
  auto tmpFile = GetTmpFile();

  auto launchArgs = MakeExecutableArgumentsForLaunch(tmpFile);
  auto injectorArgs = MakeInjectorArguments();
  auto pluginLoadDesc = MakePluginLoadDesc(tmpFile, Mode::CFunction_Single_Original3);

  auto loadArgs = MakePluginLoadArguments(launchArgs, injectorArgs, pluginLoadDesc);
  auto loadResult = Load(loadArgs);
  ASSERT_EQ(Wait(loadResult.Process), 0);

  ASSERT_STREQ(GetContent(tmpFile).c_str(), "SetUp1:Result=3:TearDown1:") << "File: " << tmpFile;
}

//
//  APP -> bifrost_add__modify_1 -> ORIGINAL
//
TEST_F(TestHook, CFunction_Single_Modify1) {
  auto tmpFile = GetTmpFile();

  auto launchArgs = MakeExecutableArgumentsForLaunch(tmpFile);
  auto injectorArgs = MakeInjectorArguments();
  auto pluginLoadDesc = MakePluginLoadDesc(tmpFile, Mode::CFunction_Single_Modify1);

  auto loadArgs = MakePluginLoadArguments(launchArgs, injectorArgs, pluginLoadDesc);
  auto loadResult = Load(loadArgs);
  ASSERT_EQ(Wait(loadResult.Process), 0);

  ASSERT_STREQ(GetContent(tmpFile).c_str(), "SetUp1:Result=7:TearDown1:") << "File: " << tmpFile;
}

//
//  APP -> bifrost_add__modify_2 -> ORIGINAL
//
TEST_F(TestHook, CFunction_Single_Modify2) {
  auto tmpFile = GetTmpFile();

  auto launchArgs = MakeExecutableArgumentsForLaunch(tmpFile);
  auto injectorArgs = MakeInjectorArguments();
  auto pluginLoadDesc = MakePluginLoadDesc(tmpFile, Mode::CFunction_Single_Modify2);

  auto loadArgs = MakePluginLoadArguments(launchArgs, injectorArgs, pluginLoadDesc);
  auto loadResult = Load(loadArgs);
  ASSERT_EQ(Wait(loadResult.Process), 0);

  ASSERT_STREQ(GetContent(tmpFile).c_str(), "SetUp1:Result=6:TearDown1:") << "File: " << tmpFile;
}

//
//  APP -> bifrost_add__modify_3 -> ORIGINAL
//
TEST_F(TestHook, CFunction_Single_Modify3) {
  auto tmpFile = GetTmpFile();

  auto launchArgs = MakeExecutableArgumentsForLaunch(tmpFile);
  auto injectorArgs = MakeInjectorArguments();
  auto pluginLoadDesc = MakePluginLoadDesc(tmpFile, Mode::CFunction_Single_Modify3);

  auto loadArgs = MakePluginLoadArguments(launchArgs, injectorArgs, pluginLoadDesc);
  auto loadResult = Load(loadArgs);
  ASSERT_EQ(Wait(loadResult.Process), 0);

  ASSERT_STREQ(GetContent(tmpFile).c_str(), "SetUp1:Result=10:TearDown1:") << "File: " << tmpFile;
}

//
//  APP -> CFunction_Single_Orignal1 -> ORIGINAL
//
TEST_F(TestHook, CFunction_Single_Replace1) {
  auto tmpFile = GetTmpFile();

  auto launchArgs = MakeExecutableArgumentsForLaunch(tmpFile);
  auto injectorArgs = MakeInjectorArguments();
  auto pluginLoadDesc = MakePluginLoadDesc(tmpFile, Mode::CFunction_Single_Replace1);

  auto loadArgs = MakePluginLoadArguments(launchArgs, injectorArgs, pluginLoadDesc);
  auto loadResult = Load(loadArgs);
  ASSERT_EQ(Wait(loadResult.Process), 0);

  ASSERT_STREQ(GetContent(tmpFile).c_str(), "SetUp1:Result=3:TearDown1:") << "File: " << tmpFile;
}

#pragma endregion

#pragma region CFunction Mutli

//
//  APP -> HookTestPlugin1:bifrost_add__original_1 -> HookTestPlugin2:bifrost_add__original_1 -> ORIGINAL
//
//TEST_F(TestHook, CFunction_Multi_Original) {
//  auto tmpFile = GetTmpFile();
//
//  auto launchArgs = MakeExecutableArgumentsForLaunch(tmpFile);
//  auto injectorArgs = MakeInjectorArguments();
//  
//  auto pluginLoadDesc1 = MakePluginLoadDesc(tmpFile, Mode::CFunction_Multi_Original_P1, 1);
//  auto pluginLoadDesc2 = MakePluginLoadDesc(tmpFile, Mode::CFunction_Multi_Original_P2, 2);
//
//  auto loadArgs = MakePluginLoadArguments(launchArgs, injectorArgs, pluginLoadDesc1, pluginLoadDesc2);
//  auto loadResult = Load(loadArgs);
//  ASSERT_EQ(Wait(loadResult1.Process), 0);
//
//  ASSERT_STREQ(GetContent(tmpFile).c_str(), "SetUp1:Result=3:TearDown1:") << "File: " << tmpFile;
//}

#pragma endregion

}  // namespace