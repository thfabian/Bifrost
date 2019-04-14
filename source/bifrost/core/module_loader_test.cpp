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

#include "bifrost/core/test.h"
#include "bifrost/core/module_loader.h"

namespace {

using namespace bifrost;

class ModuleLoaderTest : public TestBase {};

TEST_F(ModuleLoaderTest, Loader) {
  ModuleLoader loader(GetContext());

  // Load library
  HMODULE module = NULL;
  ASSERT_NO_THROW(module = loader.GetModule("user32.dll"));
  ASSERT_TRUE(module != NULL);
  ASSERT_EQ(module, loader.GetModule("user32.dll"));

  // Check names
  EXPECT_STREQ("user32.dll" , loader.GetModuleName(module).c_str());

  // Check handle
  EXPECT_EQ(module, loader.GetModule("user32.dll"));
}

}  // namespace
