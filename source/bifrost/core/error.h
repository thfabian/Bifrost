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

namespace bifrost {

/// Storage of last error message
class ErrorStash {
 public:
  ErrorStash();

  /// Get the explanation string of the last error (or NULL if no error occurred)
  const char* GetLastError() const noexcept;

  /// Set the last error message
  void SetLastError(std::string msg);

 private:
  std::string m_buffer;
};

/// Query the last WinAPI error
extern std::string GetLastWin32Error();

namespace internal {

template <bool ThrowOnError, bool HasCustomMsg, bool IsWinApi>
inline void CheckCall(Object* obj, bool cond, std::string_view msg, const char* condStr, const char* fileStr, int line) {
  if (cond == false) {
    std::string errMsg;

    if (IsWinApi) {
      errMsg = GetLastWin32Error();
    }
    if (HasCustomMsg) {
      errMsg = std::string(msg) + (errMsg.empty() ? "" : ": ") + errMsg;
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
    obj->Logger().Error(fmtMsg.c_str());

    if (ThrowOnError) {
#ifndef NDEBUG
      ::OutputDebugStringA(fmtMsg.c_str());
      if (::IsDebuggerPresent()) __debugbreak();
#endif
      throw std::runtime_error(fmtMsg.c_str());
    }
  }
}

}  // namespace internal

}  // namespace bifrost

#ifdef NDEBUG
#define BIFROST_CALL_IMPL(obj, cond, throwOnError, hasCustomMsg, isWinApi, msg) \
  ::bifrost::internal::CheckCall<throwOnError, hasCustomMsg, isWinApi>(obj, cond, msg, BIFROST_STRINGIFY(cond), nullptr, 0)
#else
#define BIFROST_CALL_IMPL(obj, cond, throwOnError, hasCustomMsg, isWinApi, msg) \
  ::bifrost::internal::CheckCall<throwOnError, hasCustomMsg, isWinApi>(obj, cond, msg, BIFROST_STRINGIFY(cond), __FILE__, __LINE__)
#endif

#define BIFROST_ASSERT_CALL(cond) BIFROST_CALL_IMPL(this, cond, true, false, false, std::string_view{})
#define BIFROST_ASSERT_CALL_MSG(cond, msg) BIFROST_CALL_IMPL(this, cond, true, true, false, msg)
#define BIFROST_ASSERT_WIN_CALL(cond) BIFROST_CALL_IMPL(this, cond, true, false, true, std::string_view{})
#define BIFROST_ASSERT_WIN_CALL_MSG(cond, msg) BIFROST_CALL_IMPL(this, cond, true, true, true, msg)

#define BIFROST_CHECK_CALL(cond) BIFROST_CALL_IMPL(this, cond, false, false, false, std::string_view{})
#define BIFROST_CHECK_CALL_MSG(cond, msg) BIFROST_CALL_IMPL(this, cond, false, true, false, msg)
#define BIFROST_CHECK_WIN_CALL(cond) BIFROST_CALL_IMPL(this, cond, false, false, true, std::string_view{})
#define BIFROST_CHECK_WIN_CALL_MSG(cond, msg) BIFROST_CALL_IMPL(this, cond, false, true, true, msg)
