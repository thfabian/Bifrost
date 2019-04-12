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

namespace bifrost {

/// C++ interface to bifrost_loader.dll - Load Plugins
///
/// Access to methods is thread-safe if the access to the underlying dll function is thread-safe.
class ApiLoader {
 public:
  ApiLoader();
  ~ApiLoader();

  struct Plugin {
    std::string Name;
    std::string DllName;
    std::string DllSearchPath;
  };

  /// Register a plugin
  void Register(const Plugin& plugin) const;

  /// Get the version of the DLL
  const char* GetVersion() const;

  /// Reset all registered plugins
  void Reset() const;

 private:
  class bfl_Api;
  std::unique_ptr<bfl_Api> m_api;
};

}  // namespace bifrost::api