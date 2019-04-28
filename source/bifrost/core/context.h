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
class SharedMemory;

/// Context object holding all persistent data
class Context {
 public:
  /// Check if a logger has been attached
  bool HasLogger() const { return m_logger != nullptr; }

  /// Get the logger
  inline ILogger& Logger() { return *m_logger; }
  inline const ILogger& Logger() const { return *m_logger; }

  /// Set the logger
  inline void SetLogger(ILogger* logger) { m_logger = logger; }

  /// Check if a shared memory has been attached
  bool HasSharedMemory() const { return m_memory != nullptr; }

  /// Get the shared memory
  inline SharedMemory& Memory() { return *m_memory; }
  inline const SharedMemory& Memory() const { return *m_memory; }

  /// Set the logger
  inline void SetMemory(SharedMemory* memory) { m_memory = memory; }

  /// Export the context by writing the necessary settings to the clipboard - returns the exported data
  std::string Export();

  /// Import the context by opening the shared memory - throws std::runtime_error on failure
  std::string Import(bool dryRun = false);

 private:
  ILogger* m_logger = nullptr;
  SharedMemory* m_memory = nullptr;
};

}  // namespace bifrost