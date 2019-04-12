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
#include "bifrost_core/logging.h"
#include "bifrost_core/api_shared.h"
#include "bifrost_core/module_loader.h"

namespace bifrost {

Logging::Logging() {
  m_buffer.resize(256);
  m_wbuffer.resize(256);
  m_module = ModuleLoader::Get().GetCurrentModuleName();
}

void Logging::SetModuleName(const char* name) { m_module = name; }

void Logging::SetCallback(const char* name, Logging::LogCallbackT loggingCallback) { api::Shared::Get().SetCallback(name, loggingCallback); }

void Logging::RemoveCallback(const char* name) { api::Shared::Get().RemoveCallback(name); }

void Logging::LogStateAsync(bool async) { api::Shared::Get().LogStateAsync(async); }

void Logging::Log(int level, const char* msg) { api::Shared::Get().Log(level, m_module.c_str(), msg); }

}  // namespace bifrost
