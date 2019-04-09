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

#include "bifrost_test_util/common.h"
#include "bifrost_test_util/logging.h"
#include "bifrost_core/util.h"
#include "bifrost_core/logging.h"
#include <iostream>

namespace bifrost::test_util {

void LogCallback(int level, const char* module, const char* msg) {
  if (level == BIFROST_LOGLEVEL_DISABLE) return;

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
    case BIFROST_LOGLEVEL_DEBUG:
      std::cout << " [DEBUG]";
      break;
    case BIFROST_LOGLEVEL_INFO:
      std::cout << " [INFO]";
      break;
    case BIFROST_LOGLEVEL_WARN:
      std::cout << " [WARN]";
      break;
    case BIFROST_LOGLEVEL_ERROR:
      std::cout << " [ERROR]";
      break;
  }

  auto moduleStr = std::string_view(module);
  if (!moduleStr.empty()) {
    std::cout << " [" << moduleStr << "]";
  }
  std::cout << ": " << msg << std::endl;
}

}  // namespace bifrost::test_util
