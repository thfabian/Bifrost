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
#include "bifrost/core/macros.h"
#include "bifrost/core/object.h"
#include "bifrost/core/exception.h"

namespace bifrost {

/// Query the last WinAPI error
extern std::string GetLastWin32Error();
extern std::string GetLastWin32Error(DWORD errorCode);

namespace internal {

template <bool ThrowOnError, bool HasCustomMsg, class MessageFuncT, class LastErrMessageFuncT>
inline void CheckCall(Context& ctx, bool cond, MessageFuncT&& msgFunc, LastErrMessageFuncT&& lastErrMsg, const char* condStr, const char* fileStr, int line) {
  CheckCall<ThrowOnError, HasCustomMsg>(&ctx, cond, std::forward<MessageFuncT>(msgFunc), std::forward<LastErrMessageFuncT>(lastErrMsg), condStr, fileStr, line);
}

template <bool ThrowOnError, bool HasCustomMsg, class MessageFuncT, class LastErrMessageFuncT>
inline void CheckCall(Context* ctx, bool cond, MessageFuncT&& msgFunc, LastErrMessageFuncT&& lastErrMsg, const char* condStr, const char* fileStr, int line) {
  if (cond == false) {
    std::string errMsg = lastErrMsg();

    if (HasCustomMsg) {
      // Use lambda function to delay evaluation of msg
      errMsg = msgFunc() + (errMsg.empty() ? "" : ": ") + errMsg;
    }

    if (errMsg.empty()) {
      errMsg = "Failed '" + std::string(condStr) + "'";
    }

    if (errMsg[errMsg.size() - 1] != '\n') {
      errMsg += '\n';
    }

#ifdef NDEBUG
    auto fmtMsg = StringFormat("%s  Cond: %s", errMsg.c_str(), condStr);
#else
    auto fmtMsg = StringFormat("%s  Cond: %s\n  File: %s:%i", errMsg.c_str(), condStr, fileStr, line);
#endif
    if (ctx->HasLogger()) {
      if (ThrowOnError) {
        ctx->Logger().Error(fmtMsg.c_str());
      } else {
        ctx->Logger().Warn(fmtMsg.c_str());
      }
    }

    if (ThrowOnError) {
#ifndef NDEBUG
      ::OutputDebugStringA(fmtMsg.c_str());
      if (::IsDebuggerPresent()) ::__debugbreak();
#endif
      throw Exception(fmtMsg);
    }
  }
}

}  // namespace internal

}  // namespace bifrost

#ifdef NDEBUG
#define BIFROST_CALL_IMPL(ctx, cond, throwOnError, hasCustomMsg, msg, lastErrMsg) \
  ::bifrost::internal::CheckCall<throwOnError, hasCustomMsg>(                     \
      ctx, cond, [&]() -> std::string { return msg; }, lastErrMsg, BIFROST_STRINGIFY(cond), nullptr, 0)
#else
#define BIFROST_CALL_IMPL(ctx, cond, throwOnError, hasCustomMsg, msg, lastErrMsg) \
  ::bifrost::internal::CheckCall<throwOnError, hasCustomMsg>(                     \
      ctx, cond, [&]() -> std::string { return msg; }, lastErrMsg, BIFROST_STRINGIFY(cond), __FILE__, __LINE__)
#endif

/// Check that the condition is true and throw an exception on error (this function needs to be called inside an bifrost::Object or any other function which
/// provides a GetContext() method)
#define BIFROST_ASSERT_CALL(cond) BIFROST_ASSERT_CALL_CTX(GetContext(), cond)
#define BIFROST_ASSERT_CALL_MSG(cond, msg) BIFROST_ASSERT_CALL_MSG_CTX(GetContext(), cond, msg)
#define BIFROST_ASSERT_WIN_CALL(cond) BIFROST_ASSERT_WIN_CALL_CTX(GetContext(), cond)
#define BIFROST_ASSERT_WIN_CALL_MSG(cond, msg) BIFROST_ASSERT_WIN_CALL_MSG_CTX(GetContext(), cond, msg)

/// Check that the condition is true and throw an exception on error
#define BIFROST_ASSERT_CALL_CTX(ctx, cond) BIFROST_CALL_IMPL(ctx, cond, true, false, {}, []() { return std::string{}; })
#define BIFROST_ASSERT_CALL_MSG_CTX(ctx, cond, msg) BIFROST_CALL_IMPL(ctx, cond, true, true, msg, []() { return std::string{}; })
#define BIFROST_ASSERT_WIN_CALL_CTX(ctx, cond) BIFROST_CALL_IMPL(ctx, cond, true, false, {}, []() { return GetLastWin32Error(); })
#define BIFROST_ASSERT_WIN_CALL_MSG_CTX(ctx, cond, msg) BIFROST_CALL_IMPL(ctx, cond, true, true, msg, []() { return GetLastWin32Error(); })

/// Check that the condition is true and issue a warning in case of an error (this function needs to be called inside an bifrost::Object or any other function
/// which provides a GetContext() method)
#define BIFROST_CHECK_CALL(cond) BIFROST_CHECK_CALL_CTX(GetContext(), cond)
#define BIFROST_CHECK_CALL_MSG(cond, msg) BIFROST_CHECK_CALL_MSG_CTX(GetContext(), cond, msg)
#define BIFROST_CHECK_WIN_CALL(cond) BIFROST_CHECK_WIN_CALL_CTX(GetContext(), cond)
#define BIFROST_CHECK_WIN_CALL_MSG(cond, msg) BIFROST_CHECK_WIN_CALL_MSG_CTX(GetContext(), cond, msg)

/// Check that the condition is true and issue a warning in case of an error
#define BIFROST_CHECK_CALL_CTX(ctx, cond) BIFROST_CALL_IMPL(ctx, cond, false, false, {}, []() { return std::string{}; })
#define BIFROST_CHECK_CALL_MSG_CTX(ctx, cond, msg) BIFROST_CALL_IMPL(ctx, cond, false, true,  msg, []() { return std::string{}; }))
#define BIFROST_CHECK_WIN_CALL_CTX(ctx, cond) BIFROST_CALL_IMPL(ctx, cond, false, false, {}, []() { return GetLastWin32Error(); })
#define BIFROST_CHECK_WIN_CALL_MSG_CTX(ctx, cond, msg) BIFROST_CALL_IMPL(ctx, cond, false, true,  msg, []() { return GetLastWin32Error(); })))
