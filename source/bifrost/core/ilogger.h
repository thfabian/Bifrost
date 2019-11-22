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

#include "bifrost/core/util.h"

namespace bifrost {

/// Logger interface
class ILogger {
 public:
  virtual ~ILogger() {}

  enum class LogLevel : u32 { Trace = 0, Debug, Info, Warn, Error, Disable };

  /// Log message at trace level
  void Trace(const char* msg) { Sink(LogLevel::Trace, msg); }
  void Trace(const wchar_t* msg) { Sink(LogLevel::Trace, WStringToString(msg).c_str()); }
  template <class... Args>
  void TraceFormat(const char* fmt, Args&&... args) {
    Sink(LogLevel::Trace, StringFormat(fmt, std::forward<Args>(args)...).c_str());
  }
  template <class... Args>
  void TraceFormat(const wchar_t* fmt, Args&&... args) {
    Sink(LogLevel::Trace, WStringToString(StringFormat(fmt, std::forward<Args>(args)...)).c_str());
  }

  /// Log message at debug level
  void Debug(const char* msg) { Sink(LogLevel::Debug, msg); }
  void Debug(const wchar_t* msg) { Sink(LogLevel::Debug, WStringToString(msg).c_str()); }
  template <class... Args>
  void DebugFormat(const char* fmt, Args&&... args) {
    Sink(LogLevel::Debug, StringFormat(fmt, std::forward<Args>(args)...).c_str());
  }
  template <class... Args>
  void DebugFormat(const wchar_t* fmt, Args&&... args) {
    Sink(LogLevel::Debug, WStringToString(StringFormat(fmt, std::forward<Args>(args)...)).c_str());
  }

  /// Log message at info level
  void Info(const char* msg) { Sink(LogLevel::Info, msg); }
  void Info(const wchar_t* msg) { Sink(LogLevel::Info, WStringToString(msg).c_str()); }
  template <class... Args>
  void InfoFormat(const char* fmt, Args&&... args) {
    Sink(LogLevel::Info, StringFormat(fmt, std::forward<Args>(args)...).c_str());
  }
  template <class... Args>
  void InfoFormat(const wchar_t* fmt, Args&&... args) {
    Sink(LogLevel::Info, WStringToString(StringFormat(fmt, std::forward<Args>(args)...)).c_str());
  }

  /// Log message at warn level
  void Warn(const char* msg) { Sink(LogLevel::Warn, msg); }
  void Warn(const wchar_t* msg) { Sink(LogLevel::Warn, WStringToString(msg).c_str()); }
  template <class... Args>
  void WarnFormat(const char* fmt, Args&&... args) {
    Sink(LogLevel::Warn, StringFormat(fmt, std::forward<Args>(args)...).c_str());
  }
  template <class... Args>
  void WarnFormat(const wchar_t* fmt, Args&&... args) {
    Sink(LogLevel::Warn, WStringToString(StringFormat(fmt, std::forward<Args>(args)...)).c_str());
  }

  /// Log message at error level
  void Error(const char* msg) { Sink(LogLevel::Error, msg); }
  void Error(const wchar_t* msg) { Sink(LogLevel::Error, WStringToString(msg).c_str()); }
  template <class... Args>
  void ErrorFormat(const char* fmt, Args&&... args) {
    Sink(LogLevel::Error, StringFormat(fmt, std::forward<Args>(args)...).c_str());
  }
  template <class... Args>
  void ErrorFormat(const wchar_t* fmt, Args&&... args) {
    Sink(LogLevel::Error, WStringToString(StringFormat(fmt, std::forward<Args>(args)...)).c_str());
  }

  /// Set the current module
  virtual void SetModule(const char* module) = 0;

  /// Sink the log message `msg` from `module`
  virtual void Sink(LogLevel level, const char* module, const char* msg) = 0;

  /// Sink the log message `msg` from the current module (set via `SetModule`)
  virtual void Sink(LogLevel level, const char* msg) = 0;
};

}  // namespace bifrost