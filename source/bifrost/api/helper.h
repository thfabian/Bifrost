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

#include "bifrost/core/common.h"

namespace bifrost::api {

#ifdef NDEBUG
#define BIFROST_API_UNCAUGHT_EXCEPTION "Uncaught exception.\n  Function:" __FUNCTION__
#else
#define BIFROST_API_UNCAUGHT_EXCEPTION "Uncaught exception.\n  Function: " __FUNCTION__ "\n  File: " __FILE__ ":" BIFROST_STRINGIFY(__LINE__)
#endif

#define BIFROST_API_CATCH_ALL_IMPL(stmts, error)            \
  try {                                                     \
    stmts;                                                  \
  } catch (std::exception & e) {                            \
    Get(ctx)->SetLastError(e.what());                       \
    return error;                                           \
  } catch (...) {                                           \
    Get(ctx)->SetLastError(BIFROST_API_UNCAUGHT_EXCEPTION); \
    return error;                                           \
  }

// Generic construction of a struct with an _Internal pointer
template <class StructT, class ClassT, class... ArgsT>
StructT* Init(ArgsT... args) {
  StructT* s = nullptr;
  try {
    s = new StructT;
    s->_Internal = new ClassT(std::forward<ArgsT>(args)...);
  } catch (...) {
  }
  return s;
}

// Generic destruction of a struct with an _Internal pointer
template <class StructT, class ClassT>
void Free(StructT* s) {
  if (s) {
    if (s->_Internal) delete (ClassT*)s->_Internal;
    s->_Internal = nullptr;
    delete s;
  }
}

}  // namespace bifrost