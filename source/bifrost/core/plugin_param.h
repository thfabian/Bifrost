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
#include "bifrost/core/type.h"

namespace bifrost {

/// Parameter for loading plugins
struct PluginLoadParam {
  struct Plugin {
    std::string Identifier;  ///< Identifier of the Plugin
    std::wstring Path;       ///< Path to the DLL
    std::string Arguments;   ///< Plugin arguments
    bool ForceLoad;          ///< Load the plugin even if it's already loaded?
  };
  std::vector<Plugin> Plugins;

  /// Serialize the parameters to a JSON string
  std::string Serialize() const;

  /// Deserialize the parameters from a JSON string - throws on error
  static PluginLoadParam Deserialize(const std::string& jStr);
};

/// Parameter for unloading plugin
struct PluginUnloadParam {
  std::vector<std::string> Plugins;  ///< List of plugins to unload
  bool UnloadAll;                    ///< Unload all plugins (ignores `Plugins` vector)?

  /// Serialize the parameters to a JSON string
  std::string Serialize() const;

  /// Deserialize the parameters from a JSON string - throws on error
  static PluginUnloadParam Deserialize(const std::string& jStr);
};

struct PluginMessageParam {
  std::string Identifier;  ///< Identifier of the plugin (as passed in `PluginLoadParam`)
  std::string Message;     ///< Message to send

  /// Serialize the parameters to a JSON string
  std::string Serialize() const;

  /// Deserialize the parameters from a JSON string - throws on error
  static PluginMessageParam Deserialize(const std::string& jStr);
};

}  // namespace bifrost