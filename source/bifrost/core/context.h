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

namespace bifrost {

class ILogger;

/// Context object holding all persistent data
class Context {
 public:
  /// Get the logger
  inline ILogger& Logger() {
    assert(m_logger);
    return *m_logger;
  }
  inline const ILogger& Logger() const {
    assert(m_logger);
    return *m_logger;
  }

  /// Set the logger
  inline void SetLogger(ILogger* logger) { m_logger = logger; }

 private:
  ILogger* m_logger;
};

}  // namespace bifrost