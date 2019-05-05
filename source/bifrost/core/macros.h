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

#define BIFROST_STRINGIFY_IMPL(x) #x

/// Convert `x` to a string constant
#define BIFROST_STRINGIFY(x) BIFROST_STRINGIFY_IMPL(x)

/// Concatinate `a` an `b`
#define BIFROST_CONCAT(a, b) a##b

/// Assert macro
#ifdef NDEBUG
#define BIFROST_ASSERT(expression) assert(expression)
#else
#define BIFROST_ASSERT(expression)                 \
  {                                                \
    if (!(expression)) {                           \
      if (::IsDebuggerPresent()) ::__debugbreak(); \
      assert(expression);                          \
    }                                              \
  }
#endif