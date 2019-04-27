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
#include "bifrost/core/error.h"

namespace bifrost {

ErrorStash::ErrorStash() {}

const char* ErrorStash::GetLastError() const noexcept { return m_buffer.empty() ? nullptr : m_buffer.c_str(); }

void ErrorStash::SetLastError(std::string msg) { m_buffer = std::move(msg); }

std::string GetLastWin32Error() { return GetLastWin32Error(::GetLastError()); }

std::string GetLastWin32Error(DWORD errorCode) {
  if (errorCode == 0) return "Unknown Error.\n";  // No error message has been recorded

  LPSTR messageBuffer = nullptr;
  size_t size = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode,
                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

  std::string message(messageBuffer, size);
  ::LocalFree(messageBuffer);
  return message;
}

}  // namespace bifrost
