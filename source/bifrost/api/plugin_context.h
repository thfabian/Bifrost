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

namespace bifrost::api {

/// Context of each plugin
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
  bfp_Status SetUp(bfp_PluginContext* ctx, const char* name, void* bfPlugin, void* setUpParam);

  struct TearDownParam {
    bool NoFail;
  };

  /// Tear-down the plugin
  bfp_Status TearDown(bfp_PluginContext* ctx, void* bfPlugin, void* tearDownParam);

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