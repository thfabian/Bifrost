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
#include "bifrost/core/type.h"

namespace bifrost {

class Context;

/// Copy the given string
template <class CharT, class TraitsT, class AllocT>
inline std::unique_ptr<CharT> StringCopy(const std::basic_string<CharT, TraitsT, AllocT>& str) {
  const std::size_t sizeInBytes = (str.size() + 1) * sizeof(CharT);
  CharT* ptr = new CharT[sizeInBytes];
  if (!ptr) return nullptr;

  std::memcpy(ptr, str.c_str(), sizeInBytes);
  return std::unique_ptr<CharT>(ptr);
}

/// Format ``fmt`` using ``args``
template <class... Args>
inline void StringFormat(std::string& str, const char* fmt, Args&&... args) {
  if (str.empty()) str.resize(std::strlen(fmt) * 2);
  if (str.size() == 0) return;

  i32 size = 0;
  while ((size = std::snprintf(str.data(), str.size(), fmt, std::forward<Args>(args)...)) < 0 || size >= str.size()) {
    str.resize(str.size() * 2);
  }
  str = str.substr(0, size);
}

/// Format ``fmt`` using ``args``
template <class... Args>
inline std::string StringFormat(const char* fmt, Args&&... args) {
  std::string str;
  StringFormat(str, fmt, std::forward<Args>(args)...);
  return str;
}

/// Format ``fmt`` using ``args``
template <class... Args>
inline void WStringFormat(std::wstring& str, const wchar_t* fmt, Args&&... args) {
  if (str.empty()) str.resize(std::wcslen(fmt) * 2);
  if (str.size() == 0) return;

  i32 size = 0;
  while ((size = _snwprintf(str.data(), str.size(), fmt, std::forward<Args>(args)...)) < 0 || size >= str.size()) {
    str.resize(str.size() * 2);
  }
  str = str.substr(0, size);
}

/// Format ``fmt`` using ``args``
template <class... Args>
inline std::wstring StringFormat(const wchar_t* fmt, Args&&... args) {
  std::wstring str;
  WStringFormat(str, fmt, std::forward<Args>(args)...);
  return str;
}

/// Convert string to wstring
extern std::wstring StringToWString(const std::string& s);

/// Convert wstring to string
extern std::string WStringToString(const std::wstring& s);

/// Get the size of the given array
template <typename T, std::size_t Size>
inline constexpr std::size_t ArraySize(T (&)[Size]) {
  return Size;
}

/// Static cast of unique ptr
template <typename U, typename T>
std::unique_ptr<U> StaticUniquePointerCast(std::unique_ptr<T>&& old) {
  return std::unique_ptr<U>{static_cast<U*>(old.release())};
}

/// Case insensitive string comparison
inline bool StringCompareCaseInsensitive(std::wstring_view str1, std::wstring_view str2) {
  return str1.size() == str2.size() && std::equal(str1.begin(), str1.end(), str2.begin(), [](wchar_t c1, wchar_t c2) -> bool {
           if (c1 == c2)
             return true;
           else if (std::towupper(c1) == std::towupper(c2))
             return true;
           return false;
         });
}
inline bool StringCompareCaseInsensitive(std::string_view str1, std::string_view str2) {
  return str1.size() == str2.size() && std::equal(str1.begin(), str1.end(), str2.begin(), [](char c1, char c2) -> bool {
           if (c1 == c2)
             return true;
           else if (std::toupper(c1) == std::toupper(c2))
             return true;
           return false;
         });
}

/// Get a UUID string
std::string UUID(Context* ctx);

}  // namespace bifrost
