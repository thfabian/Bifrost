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
#include "bifrost/core/context.h"
#include "bifrost/core/ilogger.h"
#include "bifrost/core/shared_memory.h"
#include "bifrost/core/sm_log_stash.h"

namespace bifrost::api {

#ifdef NDEBUG
#define BIFROST_API_UNCAUGHT_EXCEPTION "Uncaught exception.\n  Function:" __FUNCTION__
#else
#define BIFROST_API_UNCAUGHT_EXCEPTION "Uncaught exception.\n  Function: " __FUNCTION__ "\n  File: " __FILE__ ":" BIFROST_STRINGIFY(__LINE__)
#endif

#define BIFROST_API_CATCH_ALL_IMPL(instance, stmts, error)       \
  try {                                                     \
    stmts;                                                  \
  } catch (std::exception & e) {                            \
    Get(instance)->SetLastError(e.what());                       \
    return error;                                           \
  } catch (...) {                                           \
    Get(instance)->SetLastError(BIFROST_API_UNCAUGHT_EXCEPTION); \
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

class SharedLogger : public ILogger {
 public:
  SharedLogger(Context* ctx) : m_ctx(ctx) {}

  virtual void SetModule(const char* module) override { m_module = module; }
  virtual void Sink(LogLevel level, const char* module, const char* msg) override {
    m_ctx->Memory().GetSMLogStash()->Push(m_ctx, static_cast<u32>(level), module, msg);
  }
  virtual void Sink(LogLevel level, const char* msg) override { Sink(level, m_module.c_str(), msg); }

 private:
  Context* m_ctx;
  std::string m_module;
};

}  // namespace bifrost::api