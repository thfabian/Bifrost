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

#include "bifrost/core/common.h"
#include "bifrost/core/util.h"

namespace bifrost {

std::wstring StringToWString(const std::string& s) {
  int len;
  int slength = (int)s.length() + 1;
  len = ::MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
  std::wstring r(len, L'\0');
  ::MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, &r[0], len);
  return r;
}

extern std::string WStringToString(const std::wstring& s) {
  int len;
  int slength = (int)s.length() + 1;
  len = ::WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
  std::string r(len, '\0');
  ::WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, &r[0], len, 0, 0);
  return r;
}

}  // namespace bifrost
