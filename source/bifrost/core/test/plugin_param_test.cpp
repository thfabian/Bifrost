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

#include "bifrost/core/test/test.h"
#include "bifrost/core/module_loader.h"
#include "bifrost/core/plugin_param.h"
#include "bifrost/core/context.h"

namespace {

using namespace bifrost;

class PluginLoadParamTest : public TestBaseNoSharedMemory {};

TEST_F(PluginLoadParamTest, SerializeEmpty) {
  PluginLoadParam param;
  ASSERT_EQ(param.Plugins.size(), 0);

  PluginLoadParam other = PluginLoadParam::Deserialize(param.Serialize().c_str());
  ASSERT_EQ(param.Plugins.size(), other.Plugins.size());
}

TEST_F(PluginLoadParamTest, Serialize) {
  PluginLoadParam param;
  param.Plugins.emplace_back(PluginLoadParam::Plugin{"foo", L"bar", "foo bar"});
  ASSERT_EQ(param.Plugins.size(), 1);

  PluginLoadParam other = PluginLoadParam::Deserialize(param.Serialize().c_str());
  ASSERT_EQ(param.Plugins.size(), other.Plugins.size());
  EXPECT_STREQ(param.Plugins[0].Identifier.c_str(), other.Plugins[0].Identifier.c_str());
  EXPECT_STREQ(param.Plugins[0].Path.c_str(), other.Plugins[0].Path.c_str());
  EXPECT_STREQ(param.Plugins[0].Arguments.c_str(), other.Plugins[0].Arguments.c_str());
}

class PluginUnloadParamTest : public TestBaseNoSharedMemory {};

TEST_F(PluginUnloadParamTest, SerializeEmpty) {
  PluginUnloadParam param;
  ASSERT_EQ(param.Plugins.size(), 0);

  PluginUnloadParam other = PluginUnloadParam::Deserialize(param.Serialize().c_str());
  ASSERT_EQ(param.Plugins.size(), other.Plugins.size());
}

TEST_F(PluginUnloadParamTest, Serialize) {
  PluginUnloadParam param;
  param.Plugins.emplace_back("foo");
  param.UnloadAll = true;
  ASSERT_EQ(param.Plugins.size(), 1);

  PluginUnloadParam other = PluginUnloadParam::Deserialize(param.Serialize().c_str());
  ASSERT_EQ(param.Plugins.size(), other.Plugins.size());
  EXPECT_STREQ(param.Plugins[0].c_str(), other.Plugins[0].c_str());
  EXPECT_EQ(param.UnloadAll, other.UnloadAll);
}

class PluginMessageParamTest : public TestBaseNoSharedMemory {};

TEST_F(PluginMessageParamTest, SerializeEmpty) {
  PluginMessageParam param;

  PluginMessageParam other = PluginMessageParam::Deserialize(param.Serialize().c_str());
  ASSERT_EQ(param.Identifier, other.Identifier);
  ASSERT_EQ(param.Message, other.Message);
}

TEST_F(PluginMessageParamTest, Serialize) {
  PluginMessageParam param;
  param.Identifier = "foo";
  param.Message = "bar";

  PluginMessageParam other = PluginMessageParam::Deserialize(param.Serialize().c_str());
  ASSERT_EQ(param.Identifier, other.Identifier);
  ASSERT_EQ(param.Message, other.Message);
}

}  // namespace
