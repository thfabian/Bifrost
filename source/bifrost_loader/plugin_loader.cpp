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

#include "bifrost_loader/common.h"
#include "bifrost_loader/plugin_loader.h"
#include "bifrost_loader/bifrost_loader.h"
#include "bifrost_core/api_shared.h"

namespace bifrost::loader {

namespace {

using namespace bifrost::api;

#define PLUGIN_KEY(Idx, Key) (std::string(BFL("plugin.")) + std::to_string(Idx) + "." + std::string(Key)).c_str()

static void SetPlugin(i32 idx, const bfl_Plugin* desc) {
  assert(desc->Name);
  Shared::Get().WriteString(PLUGIN_KEY(idx, "name"), desc->Name);

  assert(desc->DllName);
  Shared::Get().WriteString(PLUGIN_KEY(idx, "dll_name"), desc->DllName);

  if (desc->DllSearchPath) {
    Shared::Get().WriteString(PLUGIN_KEY(idx, "dll_path"), desc->DllSearchPath);
  }
}

struct Plugin {
  std::string Name;
  std::string DllName;
  std::string DllSearchPath;
};

static Plugin GetPlugin(i32 idx) {
  return {Shared::Get().ReadString(PLUGIN_KEY(idx, "name")), Shared::Get().ReadString(PLUGIN_KEY(idx, "dll_name")),
          Shared::Get().ReadString(PLUGIN_KEY(idx, "dll_path"), "")};
}

static i32 GetNumPlugins() { return Shared::Get().ReadInt(BFL("plugin.count"), 0); }
static void SetNumPlugins(i32 numPlugins) { Shared::Get().WriteInt(BFL("plugin.count"), numPlugins); }

}  // namespace

std::unique_ptr<PluginLoader> PluginLoader::m_instance = nullptr;

PluginLoader::PluginLoader() {}

PluginLoader::~PluginLoader() {}

PluginLoader& PluginLoader::Get() {
  if (!m_instance) {
    m_instance = std::make_unique<PluginLoader>();
  }
  return *m_instance;
}

bool PluginLoader::LoadAllPlugins() {
  // Extract plugins

  // Load the plugins
  return true;
}

const std::vector<PluginLoader::Plugin>& PluginLoader::GetPlugins() const { return m_plugins; }

bfl_Status PluginLoader::RegisterPlugin(const bfl_Plugin* desc) const {
  auto idx = GetNumPlugins();
  SetPlugin(idx, desc);
  SetNumPlugins(idx + 1);
  return BFL_OK;
}

bfl_Status PluginLoader::Reset() {
  m_plugins.clear();
  SetNumPlugins(0);
  return BFL_OK;
}

}  // namespace bifrost::loader
