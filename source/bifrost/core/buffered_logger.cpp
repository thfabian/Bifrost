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
#include "bifrost/core/buffered_logger.h"
#include "bifrost/core/util.h"
#include <fstream>
#include <iostream>

namespace bifrost {

template <class StreamT>
static void FlushImpl(StreamT& os, const std::vector<BufferedLogger::LogMessage>& messages) {
  for (const BufferedLogger::LogMessage& msg : messages) {
    if (msg.Level == ILogger::LogLevel::Disable) return;

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    auto now_ms = now.time_since_epoch();
    auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(now_ms);
    auto tm_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now_ms - now_sec);

    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    struct tm* localTime = std::localtime(&currentTime);

    auto timeStr = StringFormat("%02i:%02i:%02i.%03i", localTime->tm_hour, localTime->tm_min, localTime->tm_sec, tm_ms.count());

    os << "[" << timeStr << "]";

    switch (msg.Level) {
      case ILogger::LogLevel::Trace:
        os << " [TRACE]";
        break;
    case ILogger::LogLevel::Debug:
        os << " [DEBUG]";
        break;
      case ILogger::LogLevel::Info:
        os << " [INFO ]";
        break;
      case ILogger::LogLevel::Warn:
        os << " [WARN ]";
        break;
      case ILogger::LogLevel::Error:
        os << " [ERROR]";
        break;
    }

    if (!msg.Module.empty()) {
      os << " [" << msg.Module << "]";
    }
    os << ": " << msg.Message << std::endl;
  }
}

void BufferedLogger::SetModule(const char* module) {
  BIFROST_LOCK_GUARD(m_mutex);
  m_module = module;
}

void BufferedLogger::Sink(LogLevel level, const char* module, const char* msg) {
  BIFROST_LOCK_GUARD(m_mutex);
  m_messages.emplace_back(LogMessage{level, module, msg});
}

void BufferedLogger::Sink(LogLevel level, const char* msg) { Sink(level, m_module.c_str(), msg); }

void BufferedLogger::Flush(ILogger* logger) {
  BIFROST_LOCK_GUARD(m_mutex);
  for (const auto& msg : m_messages) logger->Sink(msg.Level, msg.Module.c_str(), msg.Message.c_str());
  m_messages.clear();
}

bool BufferedLogger::FlushToDisk(const std::filesystem::path& path) {
  BIFROST_LOCK_GUARD(m_mutex);
  std::ofstream ofs(path.string());
  if (!ofs.is_open()) return false;
  FlushImpl(ofs, m_messages);
  ofs.close();
  m_messages.clear();
  return true;
}

bool BufferedLogger::FlushToErr() {
  BIFROST_LOCK_GUARD(m_mutex);
  FlushImpl(std::cerr, m_messages);
  m_messages.clear();
  return true;
}

const char* BufferedLogger::GetModule() {
  BIFROST_LOCK_GUARD(m_mutex);
  return m_module.c_str();
}

}  // namespace bifrost