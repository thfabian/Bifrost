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

#define BIFROST_NAMESPACE hook_plugin_2
#include "bifrost/api/test/data/plugin_def.h"

#define BIFROST_IMPLEMENTATION
#include "bifrost/template/plugin_main.h"

#define _bf_func_decl_ret_hook_plugin_2__bifrost_add bifrost_add_func_decl_ret
#define _bf_func_decl_args_hook_plugin_2__bifrost_add bifrost_add_func_decl_args
#define _bf_func_hook_plugin_2__bifrost_add bifrost_add_func

#define _bf_func_hook_plugin_2__bifrost_add bifrost_add_func
#define _bf_args_hook_plugin_2__bifrost_add bifrost_add_args

#define _bf_arg_1_hook_plugin_2__bifrost_add bifrost_add_arg_1
#define _bf_arg_2_hook_plugin_2__bifrost_add bifrost_add_arg_2

#include "bifrost/api/test/data/shared.h"

using namespace bifrost;

class HookTestPlugin2 final : public ::hook_plugin_2::Plugin {
 public:
  virtual void SetUp() override { WriteToFile(GetArguments(), "SetUp2", this); }
  virtual void TearDown() override { WriteToFile(GetArguments(), "TearDown2", this); }
};

BIFROST_REGISTER_PLUGIN(HookTestPlugin2)
