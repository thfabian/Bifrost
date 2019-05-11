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

#include "bifrost/core/common.h"
#include "bifrost/core/exception.h"
#include "bifrost/core/plugin_loader.h"
#include "bifrost/core/json.h"
#include "bifrost/core/context.h"
#include "bifrost/core/shared_memory.h"
#include "bifrost/core/sm_storage.h"

namespace bifrost {

PluginLoader::PluginLoader(Context* ctx) : Object(ctx) {}

void PluginLoader::Serialize(const std::vector<Plugin>& plugins) {
  Logger().Debug("Serializing plugins to shared memory ...");

  Json j = Json::array();
  for (const auto& plugin : plugins) {
    Json jPlugin;
    jPlugin["name"] = plugin.Name;
    jPlugin["path"] = plugin.Path;
    jPlugin["arguments"] = plugin.Arguments;
    Logger().DebugFormat(L" %-10s: \"%-20s\"%s", StringToWString(plugin.Name).c_str(), plugin.Path.c_str(),
                         plugin.Arguments.empty() ? L"" : StringFormat(L": \"%s\"", StringToWString(plugin.Arguments).c_str()).c_str());
    j.push_back(std::move(jPlugin));
  }

  GetContext().Memory().GetSMStorage()->InsertString(&GetContext(), PluginKey, j.dump());
  Logger().DebugFormat("Serialized %lu plugin(s) to shared memory", plugins.size());
}

std::vector<Plugin> PluginLoader::Deserialize() {
  std::vector<Plugin> plugins;

  Logger().Debug("Deserializing plugins from shared memory ...");
  if (!GetContext().Memory().GetSMStorage()->Contains(&GetContext(), PluginKey)) {
    Logger().Warn("No plugins deserialized");
    return plugins;
  }

  Json j;
  try {
    j = Json::parse(GetContext().Memory().GetSMStorage()->GetString(&GetContext(), PluginKey));
  } catch (JsonError& e) {
    throw Exception("Failed to parse plugins: %s", e.what());
  }

  auto throwInvalidFormat = [&](std::string msg) { throw Exception("Invalid format of plugins: " + msg + "\n\n" + j.dump(2)); };

  if (!j.is_array()) throwInvalidFormat("expected array");
  for (i32 i = 0; i < j.size(); ++i) {
    auto jPlugin = j[i];

    if (!jPlugin.count("name")) throwInvalidFormat(StringFormat("Plugin %i: missing key \"name\"", i));
    if (!jPlugin.count("path")) throwInvalidFormat(StringFormat("Plugin %i: missing key \"path\"", i));
    if (!jPlugin.count("arguments")) throwInvalidFormat(StringFormat("Plugin %i: missing key \"arguments\"", i));

    Plugin plugin;

    plugin.Name = jPlugin["name"];
    plugin.Path = jPlugin["path"].get<std::wstring>();
    plugin.Arguments = jPlugin["arguments"];
    plugins.emplace_back(std::move(plugin));
  }

  Logger().DebugFormat("Deserialized %lu plugin(s) from shared memory", plugins.size());
  return plugins;
}

void PluginLoader::Load(ModuleLoader* loader, const std::vector<Plugin>& plugins) {}

const char* PluginLoader::PluginKey = "__bifrost__.plugin";

}  // namespace bifrost
