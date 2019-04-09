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

#include "bifrost_core/config.h"
#include "bifrost_core/util.h"

namespace bifrost {

/// Logger adapter
///
/// To make use of this Logger class, you are required to register your own logging callback via `SetCallback`.
/// Use the macros `BIFROST_LOG_TRACE`, `BIFROST_LOG_INFO`, `BIFROST_LOG_WARN` and `BIFROST_LOG_ERROR` to perform the logging.
class Logging {
 public:
  /// Logging levels
  enum class LogLevel : int {
    Debug = BIFROST_LOGLEVEL_DEBUG,
    Info = BIFROST_LOGLEVEL_INFO,
    Warn = BIFROST_LOGLEVEL_WARN,
    Error = BIFROST_LOGLEVEL_ERROR,
    Disable = BIFROST_LOGLEVEL_DISABLE
  };

  /// Initialize Logger object
  ///
  /// Load bifrost_shared.dll to connect to the internal log stash
  Logging();

  /// Get singleton instance
  static Logging& Get();

  typedef void (*LogCallbackT)(int, const char*, const char*);

  /// Set the name of the module
  void SetModuleName(const char* name);

  /// Register a logging callback
  void SetCallback(const char* name, Logging::LogCallbackT loggingCallback);

  /// Remove a logging callback
  void RemoveCallback(const char* name);

  /// Set async logging off/on
  void LogStateAsync(bool async);

  /// Format `fmt` with `args` and forward the message to the loggers
  ///
  /// (!) This function is for internal use only
  template <LogLevel Level, class... Args>
  void _Log(const char* fmt, Args&&... args);
  template <LogLevel Level, class... Args>
  void _Log(const wchar_t* fmt, Args&&... args);

 private:
  void Log(int level, const char* msg);

 private:
  static std::unique_ptr<Logging> m_instance;
  std::string m_buffer;
  std::wstring m_wbuffer;

  std::string m_module;
};

template <Logging::LogLevel Level, class... Args>
void Logging::_Log(const char* fmt, Args&&... args) {
  std::memset(m_buffer.data(), 0, m_buffer.size());
  StringFormat(m_buffer, fmt, std::forward<Args>(args)...);
  Log((int)Level, m_buffer.c_str());
}

template <Logging::LogLevel Level, class... Args>
void Logging::_Log(const wchar_t* fmt, Args&&... args) {
  std::memset(m_wbuffer.data(), 0, m_wbuffer.size());
  WStringFormat(m_wbuffer, fmt, std::forward<Args>(args)...);
  Log((int)Level, WStringToString(m_wbuffer).c_str());
}

#if BIFROST_LOGLEVEL <= BIFROST_LOGLEVEL_DEBUG
#define BIFROST_LOG_DEBUG(fmt, ...) ::bifrost::Logging::Get()._Log<::bifrost::Logging::LogLevel::Debug>(fmt, __VA_ARGS__)
#else
#define BIFROST_LOG_DEBUG(fmt, ...)
#endif

#if BIFROST_LOGLEVEL <= BIFROST_LOGLEVEL_INFO
#define BIFROST_LOG_INFO(fmt, ...) ::bifrost::Logging::Get()._Log<::bifrost::Logging::LogLevel::Info>(fmt, __VA_ARGS__)
#else
#define BIFROST_LOG_INFO(fmt, ...)
#endif

#if BIFROST_LOGLEVEL <= BIFROST_LOGLEVEL_WARN
#define BIFROST_LOG_WARN(fmt, ...) ::bifrost::Logging::Get()._Log<::bifrost::Logging::LogLevel::Warn>(fmt, __VA_ARGS__)
#else
#define BIFROST_LOG_WARN(fmt, ...)
#endif

#if BIFROST_LOGLEVEL <= BIFROST_LOGLEVEL_ERROR
#define BIFROST_LOG_ERROR(fmt, ...) ::bifrost::Logging::Get()._Log<::bifrost::Logging::LogLevel::Error>(fmt, __VA_ARGS__)
#else
#define BIFROST_LOG_ERROR(fmt, ...)
#endif

}  // namespace bifrost
