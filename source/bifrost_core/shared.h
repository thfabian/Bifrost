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

namespace bifrost {

/// Shareable storage between DLLs facilitated via bifrost_shared.dll
class Shared {
 public:
  Shared();
  ~Shared();

  /// Get singleton instance
  static Shared& Get();

  /// Read the value of ``path``
  ///
  /// The value is only a *reference* to the actual value and may be modified by other users (the basic types bool, int, double are alawys atomic though). To
  /// get a copy of the data use the *Atomic* version of the function.
  /// Throws an exception in case the path does not exists or type does not match.
  /// @{
  bool ReadBool(const char* path);
  bool ReadBool(const char* path, bool default);
  bool ReadBoolAtomic(const char* path);

  int ReadInt(const char* path);
  int ReadInt(const char* path, int default);
  int ReadIntAtomic(const char* path);

  double ReadDouble(const char* path);
  double ReadDouble(const char* path, double default);
  double ReadDoubleAtomic(const char* path);

  std::string ReadString(const char* path);
  std::string ReadString(const char* path, std::string default);
  std::string ReadStringAtomic(const char* path);
  /// @}

  /// Write ``value`` to ``path``
  /// @{
  void WriteBool(const char* path, bool value);
  void WriteInt(const char* path, int value);
  void WriteDouble(const char* path, double value);
  void WriteString(const char* path, std::string value);
  /// @}

 private:
  static std::unique_ptr<Shared> m_instance;
  class bfs_Api;
  std::unique_ptr<bfs_Api> m_api;
};

}  // namespace bifrost