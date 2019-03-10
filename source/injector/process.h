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

#include "injector/common.h"

namespace injector {

class Process {
 public:
  /// Do not call directly
  Process(::STARTUPINFO&& startupInfo, ::PROCESS_INFORMATION&& procInfo);

  /// Close handles
  ~Process();

  /// Wait for the process to return the exit code
  int Wait();

  /// Launch a new process
  static std::unique_ptr<Process> Launch(const std::filesystem::path& executable, const std::vector<std::string>& args);

 private:
  ::STARTUPINFO m_startupInfo;
  ::PROCESS_INFORMATION m_procInfo;
};

}  // namespace injector