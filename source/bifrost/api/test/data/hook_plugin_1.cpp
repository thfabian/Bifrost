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

#define BIFROST_NAMESPACE hook_plugin_1
#define BIFROST_PLUGIN_IDENTIFIER
#define BIFROST_PLUGIN_STRING_TO_IDENTIFIER
#define BIFROST_PLUGIN_IDENTIFIER_TO_STRING
#define BIFROST_PLUGIN_DSL_DEF

#define BIFROST_IMPLEMENTATION
#include "bifrost/template/plugin_main.h"

#include "bifrost/api/test/data/shared.h"

class HookTestPlugin1 final : public BIFROST_PLUGIN {
 public:
  virtual void SetUp() override {}
  virtual void TearDown() override {}
};

BIFROST_REGISTER_PLUGIN(HookTestPlugin1)
