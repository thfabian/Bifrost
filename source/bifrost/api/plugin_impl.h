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

#include "bifrost/api/plugin.h"
#include "bifrost/core/buffered_logger.h"
#include "bifrost/core/context.h"
#include "bifrost/core/error.h"
#include "bifrost/core/exception.h"
#include "bifrost/core/macros.h"
#include "bifrost/core/module_loader.h"
#include "bifrost/core/shared_memory.h"

namespace bifrost::api {

class PluginManager;

/// Context of each plugin
class Plugin {
 public:
  struct InitParam {
    std::string SharedMemoryName;
    u32 SharedMemorySize;
  };

  Plugin() {
    m_ctx = std::make_unique<Context>();
    m_bufferedLogger = std::make_unique<BufferedLogger>();
    m_loader = std::make_unique<ModuleLoader>(m_ctx.get());
    m_bufferedLogger->SetModule(WStringToString(m_loader->GetCurrentModuleName()).c_str());

    SetUpBufferedLogger();
  }

  ~Plugin() {
    m_sharedLogger.reset();
    m_memory.reset();
    m_loader.reset();
    m_bufferedLogger.reset();
  }

  void SetUp(void* param) { InitParam* p = (InitParam*)param; }

  // Error stash
  void SetLastError(std::string msg) { m_error = std::move(msg); }
  const char* GetLastError() { return m_error.empty() ? "No Error" : m_error.c_str(); }

  // Logging
  void SetUpBufferedLogger() { m_ctx->SetLogger(m_bufferedLogger.get()); }
  void SetUpSharedLogger() {}

 private:
  std::string m_error;
  std::unique_ptr<Context> m_ctx;

  std::unique_ptr<SharedMemory> m_memory;
  std::unique_ptr<ModuleLoader> m_loader;

  std::unique_ptr<BufferedLogger> m_bufferedLogger;
  std::unique_ptr<SharedLogger> m_sharedLogger;
};

/// Manager of plugins
class PluginManager {
 public:
  /// Register a new plugin
  void Register(std::string name, std::unique_ptr<Plugin> plugin) { m_plugins.emplace(name, std::move(plugin)); }

  /// Get the plugin `name` or NULL
  Plugin* Get(std::string name) {
    auto it = m_plugins.find(name);
    return it != m_plugins.end() ? it->second.get() : nullptr;
  }

 private:
  std::unordered_map<std::string, std::unique_ptr<Plugin>> m_plugins;
};

}  // namespace bifrost::api