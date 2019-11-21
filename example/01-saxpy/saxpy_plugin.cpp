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

// ---------------------------------------------------------------------------------------------------
#define BIFROST_NAMESPACE saxpy
#define BIFROST_PLUGIN_IDENTIFIER saxpy,
#define BIFROST_PLUGIN_STRING_TO_IDENTIFIER {"saxpy", Plugin::Identifier::saxpy},
#define BIFROST_PLUGIN_IDENTIFIER_TO_STRING "saxpy",
#define BIFROST_PLUGIN_IDENTIFIER_TO_FUNCTION_NAME "saxpy",
#define BIFROST_PLUGIN_IDENTIFIER_TO_HOOK_TYPE HookType::CFunction,

#define BIFROST_PLUGIN_MODULE example_saxpy_dll,
#define BIFROST_PLUGIN_IDENTIFIER_TO_MODULE Module::example_saxpy_dll,
#define BIFROST_PLUGIN_MODULE_TO_STRING L"example-saxpy.dll"

#define BIFROST_PLUGIN_DSL_DEF
#define BIFROST_PLUGIN_INCLUDES
// ---------------------------------------------------------------------------------------------------

#define BIFROST_IMPLEMENTATION
#include "bifrost/template/plugin_main.h"

// ---------------------------------------------------------------------------------------------------
#define _bf_func_decl_ret_saxpy__saxpy void
#define _bf_func_decl_args_saxpy__saxpy int n, float a, float *x, float *y

#define _bf_func_saxpy__saxpy ((void (*)(int, float, float *, float *))::saxpy::Plugin::Get().GetHook<::saxpy::Plugin::Identifier::saxpy>()->GetOriginal())
#define _bf_args_saxpy__saxpy n, a, x, y

#define _bf_arg_1_saxpy__saxpy n
#define _bf_arg_2_saxpy__saxpy a
#define _bf_arg_3_saxpy__saxpy x
#define _bf_arg_4_saxpy__saxpy y
// ---------------------------------------------------------------------------------------------------

//
// 1) First, we need to define bf_id to indicate which function/method we are currently defining an override. In the following, we are going to override the
// saxpy function a couple times and finally set up a hook for one of the versions.
//
#define bf_id saxpy

//
// 2) In this override we are going to simply call the original function (i.e pass-through implementation).
//
bf_override(my_saxpy1) {
  // bf_original expands to the function pointer of the original function, bf_args expand to the arguments which have been passed to the function.
  return bf_original(bf_args);
}

bf_override(my_saxpy2) {
  // bf_call_orignal is a shorthand for the above.
  return bf_call_orignal;
}

//
// 3) Here we are going to access and modify arguments.
//
bf_override(my_saxpy3) {
  // There are several ways of accessing the arguments. You can either call them directly by the name ...
  auto a1 = a + 1;

  // ... or use bf_arg_N where N represent the index in the argument list.
  auto a2 = bf_arg_2 + 1;
  auto a3 = bf_arg(2) + 1;

  // In the end we are going to call the original function again but now with our new argument a3. Note that bf_arg can expand into multiple arguments!
  return bf_original(bf_arg(1), a3, bf_arg(3, 4));
}

//
// 4) Here we define our plugin. The plugin provides functionality to hook functions/methods and utilities such as error handling or logging. The plugin is a
// singleton and can be accessed via `Get<>()` anywhere.
//
class MySaxpyPlugin final : public ::saxpy::Plugin {
 public:
  virtual void SetUp() override {
    SetHook(Identifier::saxpy, my_saxpy1);
		SetHook("saxpy", my_saxpy2);
  }

  virtual void TearDown() override {}
};

//
// 5) Here we register our plugin - bifrost will make sure everything is defined properly.
//
BIFROST_REGISTER_PLUGIN(MySaxpyPlugin)

//
// 6) Each plugin can define a standalone help function to easily expose it's arguments.
//
const char *Help() { return "saxpy example plugin."; }
BIFROST_REGISTER_PLUGIN_HELP(Help)