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
class ISharedMemory;

/// Context object holding all persistent data
class Context {
 public:
  /// Get the logger
  inline ILogger& Logger() { return *m_logger; }
  inline const ILogger& Logger() const { return *m_logger; }

  /// Set the logger
  inline void SetLogger(ILogger* logger) { m_logger = logger; }

  /// Get the shared memory
  inline ISharedMemory& SharedMemory() { return *m_memory; }
  inline const ISharedMemory& SharedMemory() const { return *m_memory; }

  /// Set the logger
  inline void SetSharedMemory(ISharedMemory* memory) { m_memory = memory; }

 private:
  ILogger* m_logger;
  ISharedMemory* m_memory;
};

}  // namespace bifrost