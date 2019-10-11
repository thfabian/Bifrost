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
#include "bifrost/api/test/data/plugin_def.h"

#define BIFROST_DEBUG 1
#define BIFROST_IMPLEMENTATION
#include "bifrost/template/plugin_main.h"

#define _bf_func_decl_ret_hook_plugin_1__bifrost_add bifrost_add_func_decl_ret
#define _bf_func_decl_args_hook_plugin_1__bifrost_add bifrost_add_func_decl_args
#define _bf_func_hook_plugin_1__bifrost_add bifrost_add_func

#define _bf_func_hook_plugin_1__bifrost_add bifrost_add_func
#define _bf_args_hook_plugin_1__bifrost_add bifrost_add_args

#define _bf_arg_1_hook_plugin_1__bifrost_add bifrost_add_arg_1
#define _bf_arg_2_hook_plugin_1__bifrost_add bifrost_add_arg_2

#include "bifrost/api/test/data/shared.h"

using namespace bifrost;

#define bf_id bifrost_add

/// Call original function
bf_override(bifrost_add__original_1) { return bf_original(bf_args); }
bf_override(bifrost_add__original_2) { return bf_original(bf_arg(1), bf_arg(2)); }
bf_override(bifrost_add__original_3) { return bf_original(bf_arg(1, 2)); }

/// Modify arg1
bf_override(bifrost_add__modify_1) { return bf_original(5, bf_arg(2)); }

/// Modify arg2
bf_override(bifrost_add__modify_2) { return bf_original(bf_arg(1), 5); }

/// Modify both arguments
bf_override(bifrost_add__modify_3) { return bf_original(5, 5); }

class HookTestPlugin1 final : public hook_plugin_1::Plugin {
 public:
  std::string GetFile() {
    std::string args(GetArguments());
    return args.substr(0, args.find(";"));
  }
  int GetFunction() {
    std::string args(GetArguments());
    auto s = std::atoi(args.substr(args.find(";") + 1).c_str());
    return s;
  }

  virtual void SetUp() override {
    switch (GetFunction()) {
      case Function::bifrost_add__original_1:
        EnableHook(CreateHook(Identifier::bifrost_add, bifrost_add__original_1));
        break;
      case Function::bifrost_add__original_2:
        EnableHook(CreateHook(Identifier::bifrost_add, bifrost_add__original_2));
        break;
      case Function::bifrost_add__original_3:
        EnableHook(CreateHook(Identifier::bifrost_add, bifrost_add__original_3));
        break;
      case Function::bifrost_add__modify_1:
        EnableHook(CreateHook(Identifier::bifrost_add, bifrost_add__modify_1));
        break;
      case Function::bifrost_add__modify_2:
        EnableHook(CreateHook(Identifier::bifrost_add, bifrost_add__modify_2));
        break;
      case Function::bifrost_add__modify_3:
        EnableHook(CreateHook(Identifier::bifrost_add, bifrost_add__modify_3));
        break;
      default:
        break;
    }
    WriteToFile(GetFile(), "SetUp1", this);
  }

  virtual void TearDown() override { WriteToFile(GetFile(), "TearDown1", this); }
};

BIFROST_REGISTER_PLUGIN(HookTestPlugin1)
