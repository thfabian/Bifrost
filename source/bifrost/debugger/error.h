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

#include "bifrost/debugger/common.h"
#include "bifrost/core/error.h"

namespace bifrost {

/// Query the last WinAPI error
extern std::string GetLastComError(HRESULT errorCode);

#ifdef NDEBUG
#define BIFROST_COM_CALL_IMPL(ctx, hresult, throwOnError, hasCustomMsg, msg)                                                                              \
  {                                                                                                                                                       \
    auto __hr = hresult;                                                                                                                                  \
    ::bifrost::internal::CheckCall<throwOnError, hasCustomMsg>(                                                                                           \
        ctx, SUCCEEDED(__hr), [&]() -> std::string { return msg; }, [&__hr]() { return GetLastComError(__hr); }, BIFROST_STRINGIFY(hresult), nullptr, 0); \
  }
#else
#define BIFROST_COM_CALL_IMPL(ctx, hresult, throwOnError, hasCustomMsg, msg)                                                                           \
  {                                                                                                                                                    \
    auto __hr = hresult;                                                                                                                               \
    ::bifrost::internal::CheckCall<throwOnError, hasCustomMsg>(                                                                                        \
        ctx, SUCCEEDED(__hr), [&]() -> std::string { return msg; }, [&__hr]() { return GetLastComError(__hr); }, BIFROST_STRINGIFY(hresult), __FILE__, \
        __LINE__);                                                                                                                                     \
  }
#endif

/// Check that the HRESULT is SUCCEEDED and throw an exception on error
#define BIFROST_ASSERT_COM_CALL(hresult) BIFROST_ASSERT_COM_CALL_CTX(GetContext(), hresult)
#define BIFROST_ASSERT_COM_CALL_MSG(hresult, msg) BIFROST_ASSERT_COM_CALL_MSG_CTX(GetContext(), hresult, msg)
#define BIFROST_ASSERT_COM_CALL_CTX(ctx, hresult) BIFROST_COM_CALL_IMPL(ctx, hresult, true, false, {})
#define BIFROST_ASSERT_COM_CALL_MSG_CTX(ctx, hresult, msg) BIFROST_CALBIFROST_COM_CALL_IMPL(ctx, hresult, true, true, msg)

/// Check that the HRESULT is SUCCEEDED and issue a warning in case of an error
#define BIFROST_CHECK_COM_CALL(hresult) BIFROST_CHECK_COM_CALL_CTX(GetContext(), hresult)
#define BIFROST_CHECK_COM_CALL_MSG(hresult, msg) BIFROST_CHECK_COM_CALL_MSG_CTX(GetContext(), hresult, msg)
#define BIFROST_CHECK_COM_CALL_CTX(ctx, hresult, msg) BIFROST_COM_CALL_IMPL(ctx, hresult, false, false, {})
#define BIFROST_CHECK_COM_CALL_MSG_CTX(ctx, hresult, msg) BIFROST_COM_CALL_IMPL(ctx, hresult, false, true, msg)

}  // namespace bifrost
