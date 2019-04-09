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

#include "bifrost_loader/common.h"
#include "bifrost_loader/bifrost_loader.h"

namespace bifrost::loader {

/// Storage of last error message
class PluginLoader {
 public:
  PluginLoader();
  ~PluginLoader();

  /// Plugin description
  struct Plugin {
    std::string Name;
    HMODULE Module;
  };

  /// Get singleton instance
  static PluginLoader& Get();

  /// Register a plugin 
  bfl_Status RegisterPlugin(const bfl_Plugin* desc) const;

  /// Reset all registered plugins plugin
  bfl_Status Reset();

  /// Load all plugins
  bool LoadAllPlugins();

  /// Get all the loaded plugins
  const std::vector<Plugin>& GetPlugins() const;

 private:
  std::vector<Plugin> m_plugins;
  static std::unique_ptr<PluginLoader> m_instance;
};

}  // namespace bifrost::loader