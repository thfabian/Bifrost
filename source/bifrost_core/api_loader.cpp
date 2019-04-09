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

#include "bifrost_core/common.h"
#include "bifrost_core/api_loader.h"
#include "bifrost_core/module_loader.h"
#include "bifrost_core/error.h"

#include "bifrost_loader/bifrost_loader.h"  // Only for declarations

namespace bifrost::api {

class Loader::bfl_Api {
 public:
  using bfl_RegisterPlugin_fn = decltype(&bfl_RegisterPlugin);
  bfl_RegisterPlugin_fn bfl_RegisterPlugin;

  using bfl_RegisterExecutable_fn = decltype(&bfl_RegisterExecutable);
  bfl_RegisterExecutable_fn bfl_RegisterExecutable;

  using bfl_StatusString_fn = decltype(&bfl_StatusString);
  bfl_StatusString_fn bfl_StatusString;

  using bfl_GetVersion_fn = decltype(&bfl_GetVersion);
  bfl_GetVersion_fn bfl_GetVersion;

  using bfl_Reset_fn = decltype(&bfl_Reset);
  bfl_Reset_fn bfl_Reset;

  bfl_Api() {
    auto module = ModuleLoader::Get().GetModule("bifrost_loader.dll");
    BIFROST_ASSERT_WIN_CALL((bfl_RegisterPlugin = (bfl_RegisterPlugin_fn)::GetProcAddress(module, "bfl_RegisterPlugin")) != NULL);
    BIFROST_ASSERT_WIN_CALL((bfl_RegisterExecutable = (bfl_RegisterExecutable_fn)::GetProcAddress(module, "bfl_RegisterExecutable")) != NULL);
    BIFROST_ASSERT_WIN_CALL((bfl_StatusString = (bfl_StatusString_fn)::GetProcAddress(module, "bfl_StatusString")) != NULL);
    BIFROST_ASSERT_WIN_CALL((bfl_GetVersion = (bfl_GetVersion_fn)::GetProcAddress(module, "bfl_GetVersion")) != NULL);
    BIFROST_ASSERT_WIN_CALL((bfl_Reset = (bfl_Reset_fn)::GetProcAddress(module, "bfl_Reset")) != NULL);
  }
};

#define BIFROST_CHECK_BFL_CALL(call, msg, ...)                                                                                 \
  {                                                                                                                            \
    bfl_Status status = BFL_OK;                                                                                                \
    BIFROST_CHECK_CALL_MSG((status = call) == BFL_OK, StringFormat(msg ": %s", __VA_ARGS__, m_api->bfl_StatusString(status))); \
  }

std::unique_ptr<Loader> Loader::m_instance = nullptr;

Loader::Loader() { m_api = std::make_unique<bfl_Api>(); }

Loader::~Loader() = default;

Loader& Loader::Get() {
  if (!m_instance) {
    m_instance = std::make_unique<Loader>();
  }
  return *m_instance;
}

void Loader::Register(const Plugin& plugin) const {
  bfl_Plugin p;
  p.Name = plugin.Name.c_str();
  p.DllName = plugin.DllName.c_str();
  p.DllSearchPath = plugin.DllSearchPath.empty() ? NULL : plugin.DllSearchPath.c_str();
  BIFROST_CHECK_BFL_CALL(m_api->bfl_RegisterPlugin(&p), "Failed to register plugin '%s'", plugin.Name);
}

void Loader::Reset() const { BIFROST_CHECK_BFL_CALL(m_api->bfl_Reset(), "Failed to reset plugins"); }

const char* Loader::GetVersion() { return m_api->bfl_GetVersion(); }

}  // namespace bifrost::api