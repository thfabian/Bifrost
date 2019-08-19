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
#include "bifrost/core/plugin_param.h"
#include "bifrost/core/json.h"
#include "bifrost/core/context.h"

namespace bifrost {

std::string PluginLoadParam::Serialize() const {
  Json j;
  j["Plugins"] = Json::array();
  for (const auto& p : Plugins) {
    Json jPlugin;
    jPlugin["Identifier"] = p.Identifier;
    jPlugin["Path"] = p.Path;
    jPlugin["Arguments"] = p.Arguments;
    jPlugin["ForceLoad"] = p.ForceLoad;

    j["Plugins"].emplace_back(std::move(jPlugin));
  }
  return j.dump();
}

PluginLoadParam PluginLoadParam::Deserialize(const std::string& jStr) {
  PluginLoadParam param;

  if (jStr.empty()) throw Exception("Failed to parse JSON string for PluginLoadParam: JSON string is empty");

  try {
    Json j = Json::parse(jStr);

    if (!j["Plugins"].is_null()) {
      for (const auto& p : j["Plugins"]) {
        param.Plugins.emplace_back(
            PluginLoadParam::Plugin{
            p["Identifier"].get<std::string>(), 
            p["Path"].get<std::wstring>(), 
            p["Arguments"].get<std::string>(),
            p["ForceLoad"].get<bool>(),
          });
      }
    }

  } catch (std::exception& e) {
    throw Exception("Failed to parse JSON string for InjectorParam: %s\n  JSON: %s", e.what(), jStr);
  }

  return param;
}

std::string PluginUnloadParam::Serialize() const {
  Json j;
  j["Plugins"] = Plugins;
  j["UnloadAll"] = UnloadAll;
  return j.dump();
}

PluginUnloadParam PluginUnloadParam::Deserialize(const std::string& jStr) {
  PluginUnloadParam param;

  if (jStr.empty()) throw Exception("Failed to parse JSON string for PluginUnloadParam: JSON string is empty");

  try {
    Json j = Json::parse(jStr);

    if (!j["Plugins"].is_null()) {
      for (const auto& p : j["Plugins"]) param.Plugins.emplace_back(p);
    }
    param.UnloadAll = j["UnloadAll"];

  } catch (std::exception& e) {
    throw Exception("Failed to parse JSON string for InjectorParam: %s\n  JSON: %s", e.what(), jStr);
  }

  return param;
}

std::string PluginMessageParam::Serialize() const {
  Json j;
  j["Identifier"] = Identifier;
  j["Message"] = Message;
  return j.dump();
}

PluginMessageParam PluginMessageParam::Deserialize(const std::string& jStr) {
  PluginMessageParam param;

  if (jStr.empty()) throw Exception("Failed to parse JSON string for PluginMessageParam: JSON string is empty");

  try {
    Json j = Json::parse(jStr);
    param.Identifier = j["Identifier"];
    param.Message = j["Message"];
  } catch (std::exception& e) {
    throw Exception("Failed to parse JSON string for InjectorParam: %s\n  JSON: %s", e.what(), jStr);
  }
  return param;
}

}  // namespace bifrost
