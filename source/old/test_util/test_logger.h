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

#include "bifrost/core/ilogger.h"

namespace bifrost::test_util {

/// Console logger
class TestLogger : public ILogger {
 public:
  virtual void SetModule(const char* module) override;

 protected:
  virtual void Sink(LogLevel level, const char* msg) override;

 private:
  std::mutex m_mutex;
  std::string m_module;
};

extern ILogger* GetLogger();

}  // namespace bifrost::test_util