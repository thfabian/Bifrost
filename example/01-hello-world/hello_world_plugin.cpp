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

// --------------- Language Desc -----------
#define _bf_concat_impl(a, b) a##b
#define _bf_concat(a, b) _bf_concat_impl(a, b)

#define _bf_func_decl_ret _bf_concat(_bf_func_decl_ret_, bf_id)
#define _bf_func_decl_args _bf_concat(_bf_func_decl_args_, bf_id)
#define _bf_call _bf_concat(_bf_func_, bf_id)
#define _bf_args _bf_concat(_bf_args_, bf_id)

#define bf_override(name) _bf_func_decl_ret name(_bf_func_decl_args)
#define bf_call(args) _bf_call(args)
#define bf_args _bf_args

// --------------- Generated ---------------

namespace bifrost {

enum class identifer { hello_world_add };

static std::uint64_t s_OriginalAddresses[1];

}  // namespace bifrost

#define _bf_func_decl_ret_hello_world_add int
#define _bf_func_decl_args_hello_world_add int a, int b

#define _bf_func_hello_world_add ((int (*)(int, int))::bifrost::s_OriginalAddresses[(std::uint64_t)::bifrost::identifer::hello_world_add])

#define _bf_args_hello_world_add a, b

// ---------------

#define bf_id hello_world_add

/// 1) Call the original method
bf_override(my_hello_world_add1) {
  return bf_call(bf_args);
}

//
///// 2) Modify an argument
// bf_override(my_hello_world2) {
//
//  // Access the argument `b`
//  auto a1 = bf_arg_a + 1;
//
//  bf_call(a1, bf_arg_2 /*== bf_arg_b */);
//}

#undef bf_func

class MyPlugin final : public ::bifrost::Plugin {
 public:
  virtual void SetUp() override { Log(Plugin::LogLevel::Error, "Damn son!"); }
  virtual void TearDown() override { Log(Plugin::LogLevel::Error, "Doneso!"); }
};

BIFROST_REGISTER_PLUGIN(MyPlugin)