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

  /// Get the value of ``path`` (throws an exception in case the path does not exists or type does not match)
  bool GetBool(const char* path);

  /// Get the value of ``path`` or ``default`` if path does not exists
  bool GetBool(const char* path, bool default) noexcept;

 private:
  static std::unique_ptr<Shared> m_instance;
  class bfs_Api;
  std::unique_ptr<bfs_Api> m_api;
};

}  // namespace bifrost