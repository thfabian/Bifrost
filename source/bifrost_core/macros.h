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

#pragma once

#ifndef __has_feature
#define __has_feature(x) 0
#endif

#ifndef __has_extension
#define __has_extension(x) 0
#endif

#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#ifndef __has_cpp_attribute
#define __has_cpp_attribute(x) 0
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#ifndef __has_include
#define __has_include(x) 0
#endif

#if defined(__clang__)
#define BIFROST_COMPILER_CLANG 1
#endif

#if defined(__ICC) || defined(__INTEL_COMPILER)
#define BIFROST_COMPILER_INTEL 1
#endif

#if defined(__GNUC__) || defined(__GNUG__)
#define BIFROST_COMPILER_GNU 1
#endif

#if defined(_MSC_VER)
#define BIFROST_COMPILER_MSVC 1
#endif

/// Extend the default `__GNUC_PREREQ` even if glibc's `features.h` isn't available
#ifndef BIFROST_GNUC_PREREQ
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
#define BIFROST_GNUC_PREREQ(maj, min, patch) ((__GNUC__ << 20) + (__GNUC_MINOR__ << 10) + __GNUC_PATCHLEVEL__ >= ((maj) << 20) + ((min) << 10) + (patch))
#elif defined(__GNUC__) && defined(__GNUC_MINOR__)
#define BIFROST_GNUC_PREREQ(maj, min, patch) ((__GNUC__ << 20) + (__GNUC_MINOR__ << 10) >= ((maj) << 20) + ((min) << 10))
#else
#define BIFROST_GNUC_PREREQ(maj, min, patch) 0
#endif
#endif

/// Indicate unreachable state
///
/// On compilers which support it, expands to an expression which states that it is undefined behavior for the compiler to reach this point. Otherwise is not
/// defined.
#if __has_builtin(__builtin_unreachable) || BIFROST_GNUC_PREREQ(4, 5, 0)
#define BIFROST_BUILTIN_UNREACHABLE __builtin_unreachable()
#elif defined(_MSC_VER)
#define BIFROST_BUILTIN_UNREACHABLE __assume(false)
#endif

/// Mark a method as "always inline" for performance reasons
#if __has_attribute(always_inline) || BIFROST_GNUC_PREREQ(4, 0, 0)
#define BIFROST_ATTRIBUTE_ALWAYS_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
#define BIFROST_ATTRIBUTE_ALWAYS_INLINE __forceinline
#else
#define BIFROST_ATTRIBUTE_ALWAYS_INLINE
#endif

/// Mark a method as "no return"
#ifdef __GNUC__
#define BIFROST_ATTRIBUTE_NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
#define BIFROST_ATTRIBUTE_NORETURN __declspec(noreturn)
#else
#define BIFROST_ATTRIBUTE_NORETURN
#endif

/// Mark this expression as being likely evaluated to "true"
#if __has_builtin(__builtin_expect) || BIFROST_GNUC_PREREQ(4, 5, 0)
#define BIFROST_BUILTIN_LIKELY(x) __builtin_expect(!!(x), 1)
#else
#define BIFROST_BUILTIN_LIKELY(x) (x)
#endif

/// Mark this expression as being likely evaluated to "false"
#if __has_builtin(__builtin_expect) || BIFROST_GNUC_PREREQ(4, 5, 0)
#define BIFROST_BUILTIN_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define BIFROST_BUILTIN_UNLIKELY(x) (x)
#endif

/// Used to specify a minimum alignment for a structure or variable
#if __GNUC__ && !__has_feature(cxx_alignas) && !BIFROST_GNUC_PREREQ(4, 8, 1)
#define BIFROST_ALIGNAS(x) __attribute__((aligned(x)))
#else
#define BIFROST_ALIGNAS(x) alignas(x)
#endif

/// Indicate a function, variable or class is unused
///
/// Some compilers warn about unused functions. When a function is sometimes used or not depending on build settings (e.g. a function only called from  within
/// "assert"), this attribute can be used to suppress such warnings.
///
/// However, it shouldn't be used for unused *variables*, as those have a much more portable solution:
///
/// @code
///   (void)unused_var_name;
/// @endcode
///
/// Prefer cast-to-void wherever it is sufficient.
#if __has_attribute(unused) || BIFROST_GNUC_PREREQ(3, 1, 0)
#define BIFROST_ATTRIBUTE_UNUSED __attribute__((__unused__))
#else
#define BIFROST_ATTRIBUTE_UNUSED
#endif

/// Indicate a function will never return
#ifdef __GNUC__
#define BIFROST_ATTRIBUTE_NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
#define BIFROST_ATTRIBUTE_NORETURN __declspec(noreturn)
#else
#define BIFROST_ATTRIBUTE_NORETURN
#endif

/// Force inlining
#if defined(_MSC_VER)
#define BIFROST_INLINE __forceinline
#elif defined(__GNUC__)
#define BIFROST_INLINE __attribute__((always_inline))
#else
#define BIFROST_INLINE inline
#endif

/// Name of the current function
#if defined(_MSC_VER)
#define BIFROST_CURRENT_FUNCTION __FUNCSIG__
#elif defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
#define BIFROST_CURRENT_FUNCTION __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
#define BIFROST_CURRENT_FUNCTION __PRETTY_FUNCTION__
#elif defined(__FUNCSIG__)
#define BIFROST_CURRENT_FUNCTION __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#define BIFROST_CURRENT_FUNCTION __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
#define BIFROST_CURRENT_FUNCTION __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#define BIFROST_CURRENT_FUNCTION __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
#define BIFROST_CURRENT_FUNCTION __func__
#else
#define BIFROST_CURRENT_FUNCTION "(unknown)"
#endif

#define BIFROST_STRINGIFY_IMPL(x) #x

/// Convert ``x`` to a string constant
#define BIFROST_STRINGIFY(x) BIFROST_STRINGIFY_IMPL(x)
