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

namespace bifrost::api {

/// C++ interface to bifrost_loader.dll - Load Plugins
class Loader {
 public:
  Loader();
  ~Loader();

  /// Get singleton instance
  static Loader& Get();

  struct Plugin {
    std::string Name;
    std::string DllName;
    std::string DllSearchPath;
  };

  /// Register a plugin
  void Register(const Plugin& plugin) const;

  /// Get the version of the DLL
  const char* GetVersion();

  /// Reset all registered plugins
  void Reset() const;

 private:
  static std::unique_ptr<Loader> m_instance;
  class bfl_Api;
  std::unique_ptr<bfl_Api> m_api;
};

}  // namespace bifrost::api