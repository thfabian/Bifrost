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

#include "injector/common.h"

namespace injector {

class Logger {
 public:
  /// Get singleton instance
  static Logger& Get();

  /// Base-class of all sinks
  using LogSinkT = spdlog::sinks::sink;

  /// Log to file
  using FileSinkT = spdlog::sinks::basic_file_sink_mt;

  /// Log to Stdout
  using StdoutSinkT = spdlog::sinks::wincolor_stdout_sink_mt;

  /// Log to VS
  using MsvcSinkT = spdlog::sinks::msvc_sink_mt;

  /// Create an stdout color sink with customized colors
  static std::shared_ptr<StdoutSinkT> MakeStdoutSink();

  /// Create a file sink to logging to ``file``
  static std::shared_ptr<FileSinkT> MakeFileSink(const std::filesystem::path& path);

  /// Create a Visual Studio sink
  static std::shared_ptr<MsvcSinkT> MakeMsvcSink();

  /// Initialize the Logger (no sinks registered)
  Logger();

  /// Remove all sinks
  ~Logger();

  /// Log callback
  void Log(int level, const char* msg);

  /// Register the sink `name`
  void AddSink(const std::string& name, const std::shared_ptr<LogSinkT>& sink);

  /// Register the ``sinks``
  void AddSinks(const std::unordered_map<std::string, std::shared_ptr<LogSinkT>>& sinks);

  /// Unregister the sink `name`
  void RemoveSink(const std::string& name);

 private:
  /// Register the `m_logger` with `m_sinks`
  void MakeLogger();

  /// Get the underlying spd-logger
  inline std::shared_ptr<spdlog::logger>& GetLogger() noexcept { return m_logger; }

 private:
  static std::unique_ptr<Logger> m_instance;

  std::shared_ptr<spdlog::logger> m_logger;
  std::unordered_map<std::string, std::shared_ptr<LogSinkT>> m_sinks;
  std::mutex m_mutex;
};

/// Logging callback
extern void LogCallback(int level, const char* msg);

}  // namespace injector