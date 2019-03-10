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

#include "bifrost_core/common.h"
#include "bifrost_core/macros.h"
#include "bifrost_core/logging.h"

namespace bifrost {

/// Storage of last error message
class Error {
 public:
  Error();

  /// Get singleton instance
  static Error& Get();

  /// Get the explanation string of the last error (or NULL if no error occurred)
  const char* GetLastError() const noexcept;

  /// Set the last error message
  void SetLastError(std::string msg);

 private:
  static std::unique_ptr<Error> m_instance;
  std::string m_buffer;
};

/// Query the last WinAPI error
extern std::string GetLastWin32Error();

namespace internal {

template <bool ThrowOnError, bool HasCustomMsg, bool IsWinApi>
inline void CheckCall(bool cond, const char* msg, const char* condStr, const char* fileStr, int line) {
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
    BIFROST_LOG_ERROR("%s  Cond: %s", errMsg.c_str(), condStr);
#else
    BIFROST_LOG_ERROR("%s  Cond: %s\n  File: %s:%i", errMsg.c_str(), condStr, fileStr, line);
#endif

    if (ThrowOnError) {
#ifndef NDEBUG
      __debugbreak();
#endif
      throw std::runtime_error(errMsg);
    }
  }
}

}  // namespace internal

}  // namespace bifrost

#ifdef NDEBUG
#define BIFROST_CALL_IMPL(cond, throwOnError, hasCustomMsg, isWinApi, msg) \
  ::bifrost::internal::CheckCall<throwOnError, hasCustomMsg, isWinApi>(cond, msg, BIFROST_STRINGIFY(cond), nullptr, 0)
#else
#define BIFROST_CALL_IMPL(cond, throwOnError, hasCustomMsg, isWinApi, msg) \
  ::bifrost::internal::CheckCall<throwOnError, hasCustomMsg, isWinApi>(cond, msg, BIFROST_STRINGIFY(cond), __FILE__, __LINE__)
#endif

#define BIFROST_ASSERT_CALL(cond) BIFROST_CALL_IMPL(cond, true, false, false, nullptr)
#define BIFROST_ASSERT_CALL_MSG(cond, msg) BIFROST_CALL_IMPL(cond, true, true, false, msg)
#define BIFROST_ASSERT_WIN_CALL(cond) BIFROST_CALL_IMPL(cond, true, false, true, nullptr)
#define BIFROST_ASSERT_WIN_CALL_MSG(cond, msg) BIFROST_CALL_IMPL(cond, true, true, true, msg)

#define BIFROST_CHECK_CALL(cond) BIFROST_CALL_IMPL(cond, false, false, false, nullptr)
#define BIFROST_CHECK_CALL_MSG(cond, msg) BIFROST_CALL_IMPL(cond, false, true, false, msg)
#define BIFROST_CHECK_WIN_CALL(cond) BIFROST_CALL_IMPL(cond, false, false, true, nullptr)
#define BIFROST_CHECK_WIN_CALL_MSG(cond, msg) BIFROST_CALL_IMPL(cond, false, true, true, msg)
