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

#include "bifrost_core/common.h"

#include "bifrost_core/error.h"

namespace bifrost {

std::unique_ptr<Error> Error::m_instance = nullptr;

Error::Error() {}

Error& Error::Get() {
  if (!m_instance) {
    m_instance = std::make_unique<Error>();
  }
  return *m_instance;
}

const char* Error::GetLastError() const noexcept { return m_buffer.empty() ? nullptr : m_buffer.c_str(); }

void Error::SetLastError(std::string msg) { m_buffer = std::move(msg); }

std::string GetLastWin32Error() {
  DWORD errorMessageID = ::GetLastError();
  if (errorMessageID == 0) return "Unknown Error.\n";  // No error message has been recorded

  LPSTR messageBuffer = nullptr;
  size_t size = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorMessageID,
                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

  std::string message(messageBuffer, size);
  ::LocalFree(messageBuffer);
  return message;
}

}  // namespace bifrost
