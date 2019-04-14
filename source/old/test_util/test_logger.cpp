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

#include "bifrost/test_util/common.h"
#include "bifrost/test_util/test_logger.h"
#include "bifrost/core/util.h"
#include "bifrost/core/mutex.h"
#include <iostream>

namespace bifrost::test_util {

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

  std::cout << "[" << timeStr << "]";

  switch (level) {
    case ILogger::LogLevel::Debug:
      std::cout << " [DEBUG]";
      break;
    case ILogger::LogLevel::Info:
      std::cout << " [INFO]";
      break;
    case ILogger::LogLevel::Warn:
      std::cout << " [WARN]";
      break;
    case ILogger::LogLevel::Error:
      std::cout << " [ERROR]";
      break;
  }

  auto moduleStr = std::string_view(m_module);
  if (!moduleStr.empty()) {
    std::cout << " [" << moduleStr << "]";
  }
  std::cout << ": " << msg << std::endl;
}

void TestLogger::SetModule(const char* module) {
  BIFROST_LOCK_GUARD(m_mutex);
  m_module = module;
}

extern ILogger* GetLogger() {
  static TestLogger logger;
  return &logger;
}

}  // namespace bifrost::test_util
