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
#include "bifrost/core/injector_param.h"
#include "bifrost/core/json.h"
#include "bifrost/core/ilogger.h"

namespace bifrost {

std::string InjectorParam::Serialize() const {
  Json j;
  j["SharedMemoryName"] = SharedMemoryName;
  j["SharedMemorySize"] = SharedMemorySize;
  j["Pid"] = Pid;
  j["WorkingDirectory"] = WorkingDirectory;
  return j.dump();
}

InjectorParam InjectorParam::Deserialize(Context* ctx, const char* jStr) {
  InjectorParam param;
  try {
    Json j;
    j.parse(jStr);
    param.Pid = j["Pid"];
    param.WorkingDirectory = j["WorkingDirectory"].get<std::wstring>();
    param.SharedMemoryName = j["SharedMemoryName"];
    param.SharedMemorySize = j["SharedMemorySize"];

  } catch (std::exception& e) {
    ctx->Logger().ErrorFormat("Failed to parse JSON string for InjectorParam: %s\n  JSON: %s", e.what(), jStr);
  }
  return param;
}

}  // namespace bifrost