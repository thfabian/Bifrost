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
#include "bifrost/core/module_loader.h"
#include "bifrost/core/shared_memory.h"

namespace bifrost::api {

/// Context of each plugin (this is the implementation of bfp_Plugin)
class PluginContext {
 public:
  PluginContext();

  ~PluginContext();

  struct SetUpParam {
    std::string SharedMemoryName;
    u64 SharedMemorySize;
    std::string Arguments;
  };

  /// Setup the plugin
  bfp_Status SetUpStart(bfp_PluginContext* ctx, const char* name, const void* setUpParam, bfp_PluginSetUpArguments** args);
  bfp_Status SetUpEnd(bfp_PluginContext* ctx, const char* name, const void* setUpParam, const bfp_PluginSetUpArguments* args);

  struct TearDownParam {
    bool NoFail;
  };

  /// Tear-down the plugin
  bfp_Status TearDownStart(bfp_PluginContext* ctx, const void* tearDownParam, bfp_PluginTearDownArguments** args);
  bfp_Status TearDownEnd(bfp_PluginContext* ctx, const void* tearDownParam, const bfp_PluginTearDownArguments* args);

  /// Log from the plugin
  bfp_Status Log(uint32_t level, const char* module, const char* msg);

  // Error stash
  void SetLastError(std::string msg);
  const char* GetLastError();

 private:
  std::string m_error;
  std::unique_ptr<Context> m_ctx;

  std::unique_ptr<SharedMemory> m_memory;
  std::unique_ptr<ModuleLoader> m_loader;

  std::unique_ptr<BufferedLogger> m_bufferedLogger;
  std::unique_ptr<SharedLogger> m_sharedLogger;
};

}  // namespace bifrost::api