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
#include "bifrost/core/ilogger.h"
#include "bifrost/core/mutex.h"

namespace bifrost {

/// Buffer the log messages
class BufferedLogger final : public ILogger {
 public:
  struct LogMessage {
    LogLevel Level;
    std::string Module;
    std::string Message;
  };

  virtual void SetModule(const char* module) override;
  virtual void Sink(LogLevel level, const char* module, const char* msg) override;
  virtual void Sink(LogLevel level, const char* msg) override;

  /// Flush the messages to `logger`
  void Flush(ILogger* logger);

  /// Flush the message to file `path`
  bool FlushToDisk(const std::filesystem::path& path);

  /// Flush the message to stderr
  bool FlushToErr();

  /// Access the module
  const char* GetModule();

 private:
  SpinMutex m_mutex;
  std::string m_module;
  std::vector<LogMessage> m_messages;
};

}  // namespace bifrost