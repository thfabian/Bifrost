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

#include "bifrost_injector/common.h"
#include "bifrost_injector/injector.h"
#include "bifrost_injector/process.h"
#include "bifrost_core/logging.h"
#include "bifrost_core/api_shared.h"
#include "bifrost_core/api_loader.h"

namespace bifrost::injector {

Injector::Injector(std::unique_ptr<Arguments> args) : m_args(std::move(args)) {}

void Injector::InjectAndWait() {
  //// Check arguments
  // if (!executablePath) {
  //  throw std::runtime_error("No executable to inject provided: Missing argument '--exe', '--connect-exe' or '--connect-pid'\n");
  //}

  bifrost::Logging::Get().LogStateAsync(true);
  BIFROST_LOG_DEBUG("bifrost_shared: %s", api::Shared::Get().GetVersion());

  // Reset the plugin loader
  api::Loader::Get().Reset();
  BIFROST_LOG_DEBUG("bifrost_loader: %s", api::Loader::Get().GetVersion());

  //// Launch or query the process
  // std::unique_ptr<Process> process = nullptr;
  // if (executablePath) {
  //  process = Process::Launch(std::filesystem::path(executablePath.Get()), executableArg.Get());
  //}

  //// Inject the bifrost dll and wait for process to return
  //// process->inject("bifrost_loader.dll");

  // if (i32 exitCode; exitCode = process->Wait() != 0) {
  //  BIFROST_LOG_WARN("Remote process exit code: %i", exitCode);
  //}
}

}  // namespace bifrost::injector
