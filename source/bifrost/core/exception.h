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
#include "bifrost/core/util.h"

namespace bifrost {

/// Custom constructor for std::runtime_error
class Exception : public std::runtime_error {
 public:
  template <class... Args>
  Exception(const char* fmt, Args&&... args) : std::runtime_error(StringFormat(fmt, std::forward<Args>(args)...)) {}

  template <class... Args>
  Exception(const wchar_t* fmt, Args&&... args) : std::runtime_error(WStringToString(StringFormat(fmt, std::forward<Args>(args)...))) {}

  Exception(std::string_view msg) : std::runtime_error(msg.data()) {}
  Exception(std::wstring_view msg) : std::runtime_error(WStringToString(msg.data())) {}
};

}  // namespace bifrost