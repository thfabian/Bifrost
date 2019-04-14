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

#include "bifrost/core/test.h"
#include "bifrost/core/util.h"
#include "bifrost/core/mutex.h"
#include <iostream>

namespace bifrost {

void TestLogger::Sink(LogLevel level, const char* msg) {
  if (level == ILogger::LogLevel::Disable) return;

  // Get current date-time (up to ms accuracy)
  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  auto now_ms = now.time_since_epoch();
  auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(now_ms);
  auto tm_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now_ms - now_sec);

  std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
  struct tm* localTime = std::localtime(&currentTime);

  auto timeStr = StringFormat("%02i:%02i:%02i.%03i", localTime->tm_hour, localTime->tm_min, localTime->tm_sec, tm_ms.count());

  std::stringstream ss;
  ss << "[" << timeStr << "]";

  switch (level) {
    case ILogger::LogLevel::Debug:
      ss << " [DEBUG]";
      break;
    case ILogger::LogLevel::Info:
      ss << " [INFO]";
      break;
    case ILogger::LogLevel::Warn:
      ss << " [WARN]";
      break;
    case ILogger::LogLevel::Error:
      ss << " [ERROR]";
      break;
  }

  auto moduleStr = std::string_view(m_module);
  if (!moduleStr.empty()) {
    ss << " [" << moduleStr << "]";
  }
  ss << ": " << msg << std::endl;

  auto outMsg = ss.str();
  std::cerr << outMsg;
  ::OutputDebugStringA(outMsg.c_str());
}

void TestLogger::SetModule(const char* module) {
  BIFROST_LOCK_GUARD(m_mutex);
  m_module = module;
}

}  // namespace bifrost
