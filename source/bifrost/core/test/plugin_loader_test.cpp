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
#include "bifrost/core/plugin_loader.h"
#include "bifrost/core/sm_storage.h"
#include "bifrost/core/context.h"
#include "bifrost/core/shared_memory.h"

namespace {

using namespace bifrost;

class PluginLoaderTest : public TestBaseSharedMemory {};

TEST_F(PluginLoaderTest, Serialize) {
  PluginLoader loader(GetContext());

  std::vector<Plugin> plugins{{L"foo", {"1"}}, {L"bar", {"1", "2", "3"}}};
  ASSERT_NO_THROW(loader.Serialize(plugins));
  ASSERT_TRUE(GetContext()->Memory().GetSMStorage()->Contains(GetContext(), PluginLoader::PluginKey));

  std::vector<Plugin> desPlugin;

  ASSERT_NO_THROW(desPlugin = loader.Deserialize());
  ASSERT_EQ(plugins.size(), desPlugin.size());
  EXPECT_EQ(plugins[0].Path, desPlugin[0].Path);
  EXPECT_EQ(plugins[1].Path, desPlugin[1].Path);

  ASSERT_EQ(plugins[0].Arguments.size(), desPlugin[0].Arguments.size());
  EXPECT_EQ(plugins[0].Arguments[0], desPlugin[0].Arguments[0]);

  ASSERT_EQ(plugins[1].Arguments.size(), desPlugin[1].Arguments.size());
  EXPECT_EQ(plugins[1].Arguments[0], desPlugin[1].Arguments[0]);
  EXPECT_EQ(plugins[1].Arguments[1], desPlugin[1].Arguments[1]);
  EXPECT_EQ(plugins[1].Arguments[2], desPlugin[1].Arguments[2]);

  GetContext()->Memory().GetSMStorage()->InsertString(GetContext(), PluginLoader::PluginKey, "{}");
  ASSERT_THROW(loader.Deserialize(), std::runtime_error);  // root is not an array

  GetContext()->Memory().GetSMStorage()->InsertString(GetContext(), PluginLoader::PluginKey, "[{\"foo\":\"bar\"}]");
  ASSERT_THROW(loader.Deserialize(), std::runtime_error);  // missing "path" key"

  GetContext()->Memory().GetSMStorage()->InsertString(GetContext(), PluginLoader::PluginKey, "[{\"path\":\"bar\"}]");
  ASSERT_THROW(loader.Deserialize(), std::runtime_error);  // missing "arguments" key"

  GetContext()->Memory().GetSMStorage()->InsertString(GetContext(), PluginLoader::PluginKey, "[{\"path\":\"bar\", \"arguments\": \"b\"}]");
  ASSERT_THROW(loader.Deserialize(), std::runtime_error);  // "arguments" is not a list

  plugins.clear();
  ASSERT_NO_THROW(loader.Serialize(plugins));
  ASSERT_NO_THROW(desPlugin = loader.Deserialize());
  EXPECT_TRUE(desPlugin.empty());
}

}  // namespace
