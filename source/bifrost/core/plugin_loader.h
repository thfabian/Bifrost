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
#include "bifrost/core/object.h"

namespace bifrost {

class ModuleLoader;

/// Plugin description
struct Plugin {
  std::wstring Path;                   ///< Path to the DLL
  std::vector<std::string> Arguments;  ///< Plugin arguments
};

/// Load plugins (modules)
class PluginLoader : public Object {
 public:
  PluginLoader(Context* ctx);

  /// Serialize plugins to shared memory - overrides previously serialized plugins
  void Serialize(const std::vector<Plugin>& plugins);

  /// Deserialize the plugins (returns an empty vector if there are no plugins)
  std::vector<Plugin> Deserialize();

  /// Load plugins
  void Load(ModuleLoader* loader, const std::vector<Plugin>& plugins);

  /// Pluign key
  static const char* PluginKey;
};

}  // namespace bifrost