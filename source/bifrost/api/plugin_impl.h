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

#include "bifrost/api/helper.h"
#include "bifrost/api/plugin.h"
#include "bifrost/core/buffered_logger.h"
#include "bifrost/core/context.h"
#include "bifrost/core/error.h"
#include "bifrost/core/exception.h"
#include "bifrost/core/macros.h"
#include "bifrost/core/module_loader.h"
#include "bifrost/core/shared_memory.h"

#define BIFROST_IMPLEMENTATION
#include "bifrost/template/plugin_decl.h"

namespace bifrost::api {

/// Context of each plugin
class PluginContext {
 public:
  PluginContext() {
    m_ctx = std::make_unique<Context>();
    m_bufferedLogger = std::make_unique<BufferedLogger>();
    m_loader = std::make_unique<ModuleLoader>(m_ctx.get());
    m_bufferedLogger->SetModule(WStringToString(m_loader->GetCurrentModuleName()).c_str());
    m_ctx->SetLogger(m_bufferedLogger.get());
  }

  ~PluginContext() {
    m_sharedLogger.reset();
    m_memory.reset();
    m_loader.reset();
    m_bufferedLogger.reset();
  }

  struct SetUpParam {
    std::string SharedMemoryName;
    u64 SharedMemorySize;
  };

  bfp_Status SetUp(bfp_PluginContext* ctx, const char* name, void* bfPlugin, void* setUpParam) {
    Plugin* plugin = (Plugin*)bfPlugin;
    SetUpParam* param = (SetUpParam*)setUpParam;
    try {
      // Set the module
      m_bufferedLogger->SetModule(name);
      m_ctx->Logger().InfoFormat("Initializing plugin: %s", name);

      // Connect to the shared memory
      m_memory = std::make_unique<SharedMemory>(m_ctx.get(), param->SharedMemoryName, param->SharedMemorySize);
      m_ctx->SetMemory(m_memory.get());

      // Flush the buffered logger and start logging to shared memory
      m_sharedLogger = std::make_unique<SharedLogger>(m_ctx.get());
      m_sharedLogger->SetModule(name);
      m_ctx->SetLogger(m_sharedLogger.get());
      m_bufferedLogger->Flush(m_sharedLogger.get());

      // Call the setup method
      plugin->_SetUpImpl((bfp_PluginContext_t*)ctx);

    } catch (...) {
      m_ctx->Logger().ErrorFormat("Failed to initialize plugin: %s", name);
      throw;
    }
    return BFP_OK;
  }

  struct TearDownParam {
    bool NoFail;
  };

  bfp_Status TearDown(bfp_PluginContext* ctx, void* bfPlugin, void* tearDownParam) { return BFP_OK; }

  bfp_Status Log(uint32_t level, const char* module, const char* msg) {
    m_ctx->Logger().Sink((ILogger::LogLevel)level, module, msg);
    return BFP_OK;
  }

  // Error stash
  void SetLastError(std::string msg) { m_error = std::move(msg); }
  const char* GetLastError() { return m_error.empty() ? "No Error" : m_error.c_str(); }

 private:
  std::string m_error;
  std::unique_ptr<Context> m_ctx;

  std::unique_ptr<SharedMemory> m_memory;
  std::unique_ptr<ModuleLoader> m_loader;

  std::unique_ptr<BufferedLogger> m_bufferedLogger;
  std::unique_ptr<SharedLogger> m_sharedLogger;
};

}  // namespace bifrost::api