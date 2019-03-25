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
#include "bifrost_core/logging.h"

namespace bifrost {

std::unique_ptr<Logging> Logging::m_instance = nullptr;

Logging::Logging() : m_enabled(true) { m_buffer.resize(256); }

Logging& Logging::Get() {
  if (!m_instance) {
    m_instance = std::make_unique<Logging>();
  }
  return *m_instance;
}

void Logging::Enable(bool enable) { m_enabled = enable; }

void Logging::SetCallback(const char* name, LogCallBackT loggingCallback) { m_loggingCallback[name] = loggingCallback; }

void Logging::RemoveCallback(const char* name) { m_loggingCallback.erase(name); }

const Logging::LogCallBackT Logging::GetCallback(const char* name) const {
  auto it = m_loggingCallback.find(name);
  return it != m_loggingCallback.end() ? it->second : nullptr;
}

Logging::LogLevel Logging::GetMinLogLevel() const noexcept { return static_cast<LogLevel>(BIFROST_LOGLEVEL); }

}  // namespace bifrost
