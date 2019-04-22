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
#include "bifrost/core/object.h"
#include "bifrost/core/type.h"

namespace bifrost {

/// Process creation abstraction
class Process : public Object {
 public:
  struct LaunchArguments {
    std::filesystem::path Executable;    ///< Path to the executable
    std::vector<std::string> Arguments;  ///< Arguments passed to the executable;
    bool Suspended = false;              ///< Launch suspended?
  };

  /// Launch the process using `args` - throws std::runtime_error on failure
  Process(Context* ctx, LaunchArguments args);
  ~Process();

  Process(Process&&) = default;
  Process& operator=(Process&&) = default;

  /// Resume the process after suspension - throws std::runtime_error on failure
  void Resume();

  /// Wait for the process to return (blocks the calling thread) and return it's exit code
  void Wait();

  /// Poll the launched process and return true if the process has exited
  bool Poll();

  /// Return the exit code or NULL if the process has not yet exited
  const i32* ExitCode();

 private:
  bool TrySetExitCode();

 private:
  LaunchArguments m_args;
  ::STARTUPINFO m_startupInfo;
  ::PROCESS_INFORMATION m_procInfo;
  std::optional<i32> m_exitCode;
};

}  // namespace bifrost