#pragma region Bifrost DSL

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

#ifndef BIFROST_PLUGIN
namespace bifrost {

enum class Identifer : std::uint64_t { Unsused = 0, NumIdentifier };

}  // namespace bifrost
#endif

#pragma endregion