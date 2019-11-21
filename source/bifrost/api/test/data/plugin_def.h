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

//
// This file contains some shorthands for defining the hook DLS by hand which would normally be done by the bifrost compiler.
//

#define BIFROST_PLUGIN_IDENTIFIER bifrost_add, bifrost_Adder_add,
#define BIFROST_PLUGIN_STRING_TO_IDENTIFIER \
  {"bifrost_add", Plugin::Identifier::bifrost_add}, { "bifrost_Adder_add", Plugin::Identifier::bifrost_Adder_add }
#define BIFROST_PLUGIN_IDENTIFIER_TO_STRING "bifrost_add", "bifrost_Adder_add",
#define BIFROST_PLUGIN_IDENTIFIER_TO_FUNCTION_NAME "bifrost_add", "",
#define BIFROST_PLUGIN_IDENTIFIER_TO_HOOK_TYPE HookType::CFunction, HookType::VTable,

#define BIFROST_PLUGIN_MODULE test_bifrost_api_hook_dll_dll,
#define BIFROST_PLUGIN_MODULE_TO_STRING L"test-bifrost-api-hook-dll.dll",
#define BIFROST_PLUGIN_IDENTIFIER_TO_MODULE Module::test_bifrost_api_hook_dll_dll, Module::test_bifrost_api_hook_dll_dll,

#define BIFROST_PLUGIN_INCLUDES
#include "hook_dll.h"

#define BIFROST_PLUGIN_DSL_DEF

// Use the following shorthands to define the DSL in hook_plugin_1/2.dll

// bifrost_add
#define bifrost_add_func_decl_ret int
#define bifrost_add_func_decl_args int arg1, int arg2
#define bifrost_add_func \
  ((int (*)(int, int))BIFROST_NAMESPACE## ::Plugin::Get().GetHook<BIFROST_NAMESPACE## ::Plugin::Identifier::bifrost_add>()->GetOriginal())

#define bifrost_add_args arg1, arg2
#define bifrost_add_arg_1 arg1
#define bifrost_add_arg_2 arg2

// bifrost::Adder::add
#define bifrost_Adder_add_func_decl_ret int
#define bifrost_Adder_add_func_decl_args bifrost::Adder *__this__, int arg1, int arg2
#define bifrost_Adder_add_func                                                 \
  ((int (*)(bifrost::Adder *, int, int))BIFROST_NAMESPACE## ::Plugin::Get()    \
       .GetHook<BIFROST_NAMESPACE## ::Plugin::Identifier::bifrost_Adder_add>() \
       ->GetOriginal())

#define bifrost_Adder_add_this __this__
#define bifrost_Adder_add_args arg1, arg2
#define bifrost_Adder_add_arg_1 arg1
#define bifrost_Adder_add_arg_2 arg2

namespace bifrost {

enum class Mode : int {
  none = 0,
  bifrost_add__original_1,
  bifrost_add__original_2,
  bifrost_add__original_3,
  bifrost_add__modify_1,
  bifrost_add__modify_2,
  bifrost_add__modify_3,
  bifrost_add__replace_1,

  bifrost_Adder_add__original_1,
};

}