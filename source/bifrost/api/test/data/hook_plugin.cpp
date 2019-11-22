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

#include "hook_plugin.h"

#ifndef PLUGIN_NAME
#define PLUGIN_NAME HookTestPlugin
#endif

#define BIFROST_NAMESPACE hook_plugin

#define BIFROST_PLUGIN_IDENTIFIER bifrost_add, bifrost_Adder_add,
#define BIFROST_PLUGIN_IDENTIFIER_TO_STRING "bifrost_add", "bifrost_Adder_add",
#define BIFROST_PLUGIN_IDENTIFIER_TO_FUNCTION_NAME "bifrost_add", "",
#define BIFROST_PLUGIN_IDENTIFIER_TO_HOOK_TYPE HookType::CFunction, HookType::VTable,
#define BIFROST_PLUGIN_STRING_TO_IDENTIFIER \
  {"bifrost_add", Plugin::Identifier::bifrost_add}, { "bifrost_Adder_add", Plugin::Identifier::bifrost_Adder_add }

#define BIFROST_PLUGIN_MODULE test_bifrost_api_hook_dll_dll,
#define BIFROST_PLUGIN_MODULE_TO_STRING L"test-bifrost-api-hook-dll.dll",
#define BIFROST_PLUGIN_IDENTIFIER_TO_MODULE Module::test_bifrost_api_hook_dll_dll, Module::test_bifrost_api_hook_dll_dll,

#define BIFROST_PLUGIN_INCLUDES
#include "hook_dll.h"

#define BIFROST_DEBUG 1
#define BIFROST_IMPLEMENTATION
#define BIFROST_PLUGIN_DSL_DEF

#include "bifrost/template/plugin_main.h"

//
// Generated DSL
//

// bifrost_add
#define _bf_func_decl_ret_hook_plugin__bifrost_add int
#define _bf_func_decl_args_hook_plugin__bifrost_add int arg1, int arg2
#define _bf_func_hook_plugin__bifrost_add \
  ((int (*)(int, int))BIFROST_NAMESPACE## ::Plugin::Get().GetHook<BIFROST_NAMESPACE## ::Plugin::Identifier::bifrost_add>()->GetOriginal())

#define _bf_args_hook_plugin__bifrost_add arg1, arg2
#define _bf_arg_1_hook_plugin__bifrost_add arg1
#define _bf_arg_2_hook_plugin__bifrost_add arg2

// bifrost::Adder::add
#define _bf_func_decl_ret_hook_plugin__bifrost_Adder_add int
#define _bf_func_decl_args_hook_plugin__bifrost_Adder_add bifrost::Adder *__this__, int arg1, int arg2
#define _bf_func_hook_plugin__bifrost_Adder_add                                \
  ((int (*)(bifrost::Adder *, int, int))BIFROST_NAMESPACE## ::Plugin::Get()    \
       .GetHook<BIFROST_NAMESPACE## ::Plugin::Identifier::bifrost_Adder_add>() \
       ->GetOriginal())

#define _bf_this_hook_plugin__bifrost_Adder_add __this__
#define _bf_args_hook_plugin__bifrost_Adder_add arg1, arg2
#define _bf_arg_1_hook_plugin__bifrost_Adder_add arg1
#define _bf_arg_2_hook_plugin__bifrost_Adder_add arg2

#include "bifrost/api/test/data/shared.h"

using namespace bifrost;

//
// bifrost_add
//
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

#undef bf_id

//
// bifrost_Adder_add
//
#define bf_id bifrost_Adder_add

/// Call original function
// bf_override(bifrost_Adder_add__original_1) { return bf_original(bf_this, bf_args); }

#undef bf_id

class PLUGIN_NAME final : public hook_plugin::Plugin {
 public:
  // Get the file <mode> from "<file>;<mode>"
  std::string GetFile() {
    std::string args(GetArguments());
    return args.substr(0, args.find(";"));
  }

  // Get the file <mode> from "<file>;<mode>"
  Mode GetMode() {
    std::string args(GetArguments());
    auto s = std::atoi(args.substr(args.find(";") + 1).c_str());
    return (Mode)s;
  }

  virtual void SetUp() override {
    switch (GetMode()) {
      case Mode::CFunction_Single_Orignal1:
        SetHook(Identifier::bifrost_add, bifrost_add__original_1);
        break;
      case Mode::CFunction_Single_Orignal2:
        SetHook(Identifier::bifrost_add, bifrost_add__original_2);
        break;
      case Mode::CFunction_Single_Orignal3:
        SetHook(Identifier::bifrost_add, bifrost_add__original_3);
        break;
      case Mode::CFunction_Single_Modify1:
        SetHook(Identifier::bifrost_add, bifrost_add__modify_1);
        break;
      case Mode::CFunction_Single_Modify2:
        SetHook(Identifier::bifrost_add, bifrost_add__modify_2);
        break;
      case Mode::CFunction_Single_Modify3:
        SetHook(Identifier::bifrost_add, bifrost_add__modify_3);
        break;
      case Mode::CFunction_Single_Replace1:
        SetHook(Identifier::bifrost_add, bifrost_add__modify_3);
        SetHook(Identifier::bifrost_add, bifrost_add__original_1);
        break;
      default:
        break;
    }
    WriteToFile(GetFile(), "SetUp1", this);
  }

  virtual void TearDown() override { WriteToFile(GetFile(), "TearDown1", this); }
};

BIFROST_REGISTER_PLUGIN(PLUGIN_NAME)
