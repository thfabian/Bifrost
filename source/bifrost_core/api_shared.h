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

#include "bifrost_core/common.h"
#include "bifrost_core/type.h"
#include "bifrost_core/logging.h"

namespace bifrost {

/// C++ interface to bifrost_shared.dll - Storage between DLLs
///
/// Access to methods is thread-safe if the access to the underlying dll function is thread-safe.
class ApiShared {
 public:
  ApiShared();
  ~ApiShared();

  /// Read the value of ``path``
  ///
  /// The value is only a *reference* to the actual value and may be modified by other users (the basic types bool, int, double are alawys atomic though). To
  /// get a copy of the data use the *Atomic* version of the function.
  /// Throws an exception in case the path does not exists or type does not match.
  /// @{
  bool ReadBool(const char* path) const;
  bool ReadBool(const char* path, bool default) const;
  bool ReadBoolAtomic(const char* path) const;

  i32 ReadInt(const char* path) const;
  i32 ReadInt(const char* path, i32 default) const;
  i32 ReadIntAtomic(const char* path) const;

  double ReadDouble(const char* path) const;
  double ReadDouble(const char* path, double default) const;
  double ReadDoubleAtomic(const char* path) const;

  std::string ReadString(const char* path) const;
  std::string ReadString(const char* path, std::string default) const;
  std::string ReadStringAtomic(const char* path) const;
  /// @}

  /// Write ``value`` to ``path``
  /// @{
  void WriteBool(const char* path, bool value) const;
  void WriteInt(const char* path, i32 value) const;
  void WriteDouble(const char* path, double value) const;
  void WriteString(const char* path, std::string value) const;
  /// @}

  /// Allocate ``size`` memory
  void* Alloc(u32 size) const;

  /// Deallocate ``ptr``
  void Deallocate(void* ptr) const;

  /// Push a log message
  void Log(i32 level, const char* module, const char* message) const;

  /// Register a logging callback
  void SetCallback(const char* name, Logging::LogCallbackT loggingCallback) const;

  /// Remove a logging callback
  void RemoveCallback(const char* name) const;

  /// Set async logging off/on
  void LogStateAsync(bool async) const;

  /// Get the version of the DLL
  const char* GetVersion() const;

  /// Reset all shared memory
  void Reset() const;

 private:
  class bfs_Api;
  std::unique_ptr<bfs_Api> m_api;
};

}  // namespace bifrost::api