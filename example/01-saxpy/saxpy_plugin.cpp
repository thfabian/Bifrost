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

#define BIFROST_IMPLEMENTATION
#include "bifrost/template/plugin_main.h"

/********************************************/

#define _bf_concat_impl(a, b) a##b
#define _bf_concat(a, b) _bf_concat_impl(a, b)

// Implementation of bf_arg
#define _bf_indirect_expand(m, args) m args

#define _bf_concate_(X, Y) X##Y
#define _bf_concate(X, Y) _bf_concate_(X, Y)

#define _bf_num_args2(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, TOTAL, ...) TOTAL
#define _bf_num_args_(...) _bf_indirect_expand(_bf_num_args2, (__VA_ARGS__))
#define _bf_num_args(...) _bf_num_args_(__VA_ARGS__, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define _bf_var_macro(MACRO, ...) _bf_indirect_expand(_bf_concate, (MACRO, _bf_num_args(__VA_ARGS__)))(__VA_ARGS__)

#define _bf_arg(...) _bf_var_macro(_bf_arg, __VA_ARGS__)
#define _bf_arg_idx(idx) _bf_concat(_bf_concat(_bf_arg_, idx), _bf_concat(_, bf_id))
#define _bf_arg1(a1) _bf_arg_idx(a1)
#define _bf_arg2(a1, a2) _bf_arg1(a1), _bf_arg_idx(a2)
#define _bf_arg3(a1, a2, a3) _bf_arg2(a1, a2), _bf_arg_idx(a3)
#define _bf_arg4(a1, a2, a3, a4) _bf_arg3(a1, a2, a3), _bf_arg_idx(a4)
#define _bf_arg5(a1, a2, a3, a4, a5) _bf_arg4(a1, a2, a3, a4), _bf_arg_idx(a5)
#define _bf_arg6(a1, a2, a3, a4, a5, a6) _bf_arg5(a1, a2, a3, a4, a5), _bf_arg_idx(a6)
#define _bf_arg7(a1, a2, a3, a4, a5, a6, a7) _bf_arg6(a1, a2, a3, a4, a5, a6), _bf_arg_idx(a7)
#define _bf_arg8(a1, a2, a3, a4, a5, a6, a7, a8) _bf_arg7(a1, a2, a3, a4, a5, a6, a7), _bf_arg_idx(a8)
#define _bf_arg9(a1, a2, a3, a4, a5, a6, a7, a8, a9) _bf_arg8(a1, a2, a3, a4, a5, a6, a7, a8), _bf_arg_idx(a9)
#define _bf_arg10(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) _bf_arg9(a1, a2, a3, a4, a5, a6, a7, a8, a9), _bf_arg_idx(a10)
#define _bf_arg11(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) _bf_arg10(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10), _bf_arg_idx(a11)
#define _bf_arg12(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) _bf_arg11(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11), _bf_arg_idx(a12)

// Implementation of bf_override
#define _bf_func_decl_ret _bf_concat(_bf_func_decl_ret_, bf_id)
#define _bf_func_decl_args _bf_concat(_bf_func_decl_args_, bf_id)

// Implementation of bf_original
#define _bf_original _bf_concat(_bf_func_, bf_id)

// Implementation of bf_args
#define _bf_args _bf_concat(_bf_args_, bf_id)

/// bf_override
#define bf_override(name) _bf_func_decl_ret name(_bf_func_decl_args)

/// bf_original
#define bf_original(...) _bf_original(__VA_ARGS__)

/// bf_args
#define bf_args _bf_args

/// bf_arg
#define bf_arg(...) _bf_arg(__VA_ARGS__)

#define bf_arg_1 _bf_arg(1)
#define bf_arg_2 _bf_arg(2)
#define bf_arg_3 _bf_arg(3)
#define bf_arg_4 _bf_arg(4)
#define bf_arg_5 _bf_arg(5)
#define bf_arg_6 _bf_arg(6)
#define bf_arg_7 _bf_arg(7)
#define bf_arg_8 _bf_arg(8)
#define bf_arg_9 _bf_arg(9)
#define bf_arg_10 _bf_arg(10)
#define bf_arg_11 _bf_arg(11)
#define bf_arg_12 _bf_arg(12)
#define bf_arg_13 _bf_arg(13)
#define bf_arg_14 _bf_arg(14)
#define bf_arg_15 _bf_arg(15)
#define bf_arg_16 _bf_arg(16)
#define bf_arg_17 _bf_arg(17)
#define bf_arg_18 _bf_arg(18)
#define bf_arg_19 _bf_arg(19)
#define bf_arg_20 _bf_arg(20)
#define bf_arg_21 _bf_arg(21)
#define bf_arg_22 _bf_arg(22)
#define bf_arg_23 _bf_arg(23)
#define bf_arg_24 _bf_arg(24)
#define bf_arg_25 _bf_arg(25)
#define bf_arg_26 _bf_arg(26)
#define bf_arg_27 _bf_arg(27)
#define bf_arg_28 _bf_arg(28)
#define bf_arg_29 _bf_arg(29)
#define bf_arg_30 _bf_arg(30)

/// bf_arg_name
#define bf_arg_name(index)

/// bf_arg_type
#define bf_arg_name(index)

#define bf_arg_name(index)
#define bf_arg_name(index)
#define bf_arg_name(index)

/// bf_call_original
#define bf_call_orignal bf_original(bf_args)

#ifdef __INTELLISENSE__
#undef bf_original
#define bf_original(...)
#endif

// --------------- Generated ---------------

enum class identifer { saxpy };

static std::uint64_t s_OriginalAddresses[1];

// saxpy
#define _bf_func_decl_ret_saxpy void
#define _bf_func_decl_args_saxpy int n, float a, float *x, float *y

#define _bf_func_saxpy ((void (*)(int, float, float *, float *))::s_OriginalAddresses[(std::uint64_t)::identifer::saxpy])

#define _bf_args_saxpy n, a, x, y

#define _bf_arg_1_saxpy n
#define _bf_arg_2_saxpy a
#define _bf_arg_3_saxpy x
#define _bf_arg_4_saxpy y

// ---------------

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
class MySaxpyPlugin final : public ::bifrost::Plugin {
 public:
  virtual void SetUp() override {
    // bf_hook(hello_world_add, hello_world_add3)
  }

  virtual void TearDown() override {}
};

//
// 5) Here we register our plugin - bifrost will make sure everything.
//
BIFROST_REGISTER_PLUGIN(MySaxpyPlugin)

//
// 6) Each plugin can define a standalone help function to easily expose it's arguments.
//
const char *Help() { return "saxpy example plugin."; }
BIFROST_REGISTER_PLUGIN_HELP(Help)