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

#include "bifrost/api/plugin_context.h"

#include "bifrost/template/plugin_namespace.h"
#include "bifrost/template/plugin_interface.h"
#include "bifrost/template/plugin_interface_priv.h"
#include "bifrost/template/plugin_procedure.h"

namespace bifrost::api {

bfp_Status PluginContext::SetUp(bfp_PluginContext* ctx, const char* name, void* bfPlugin, void* setUpParam) {
  Plugin* plugin = (Plugin*)bfPlugin;
  SetUpParam* param = (SetUpParam*)setUpParam;
  try {
    // Set the module
    m_bufferedLogger->SetModule(name);
    m_ctx->Logger().InfoFormat("Setting up plugin: %s", name);

    // Connect to the shared memory
    m_memory = std::make_unique<SharedMemory>(m_ctx.get(), param->SharedMemoryName, param->SharedMemorySize);
    m_ctx->SetMemory(m_memory.get());

    // Flush the buffered logger and start logging to shared memory
    m_sharedLogger = std::make_unique<SharedLogger>(m_ctx.get());
    m_sharedLogger->SetModule(name);
    m_ctx->SetLogger(m_sharedLogger.get());
    m_bufferedLogger->Flush(m_sharedLogger.get());

    // Set the arguments
    plugin->_SetArguments(param->Arguments.empty() ? "" : param->Arguments.c_str());

    // Call the setup method
    plugin->_SetUpImpl((bfp_PluginContext_t*)ctx);

  } catch (...) {
    m_ctx->Logger().ErrorFormat("Failed to set up plugin: %s", name);
    throw;
  }
  return BFP_OK;
}

bfp_Status PluginContext::TearDown(bfp_PluginContext* ctx, void* bfPlugin, void* tearDownParam) {
  Plugin* plugin = (Plugin*)bfPlugin;
  TearDownParam* param = (TearDownParam*)tearDownParam;
  std::string name = m_bufferedLogger->GetModule();

  try {
    m_ctx->Logger().InfoFormat("Tearing down plugin: %s", name.c_str());

    // Call tear-down
    plugin->_TearDownImpl(param->NoFail);

    // Remove the shared logger
    m_sharedLogger.reset();
    m_ctx->SetLogger(m_bufferedLogger.get());

    // Disconnect the shared memory
    m_memory.reset();
    m_ctx->SetMemory(nullptr);

  } catch (...) {
    m_ctx->Logger().ErrorFormat("Failed to tear down plugin: %s", name.c_str());
    throw;
  }
  return BFP_OK;
}

bfp_Status PluginContext::Log(uint32_t level, const char* module, const char* msg) {
  m_ctx->Logger().Sink((ILogger::LogLevel)level, module, msg);
  return BFP_OK;
}

PluginContext::~PluginContext() {
  m_sharedLogger.reset();
  m_memory.reset();
  m_loader.reset();
  m_bufferedLogger.reset();
}

const char* PluginContext::GetLastError() { return m_error.empty() ? "No Error" : m_error.c_str(); }

void PluginContext::SetLastError(std::string msg) { m_error = std::move(msg); }

PluginContext::PluginContext() {
  m_ctx = std::make_unique<Context>();
  m_bufferedLogger = std::make_unique<BufferedLogger>();
  m_loader = std::make_unique<ModuleLoader>(m_ctx.get());
  m_bufferedLogger->SetModule(WStringToString(m_loader->GetCurrentModuleName()).c_str());
  m_ctx->SetLogger(m_bufferedLogger.get());
}

}  // namespace bifrost::api