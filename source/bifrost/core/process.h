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

  /// Connect to the process given by PID
  Process(Context* ctx, u32 pid);

  /// Connect to the process `name`
  Process(Context* ctx, std::wstring_view name);

  Process(Process&&) = default;
  Process& operator=(Process&&) = default;
  ~Process();

  /// Resume the process after suspension - throws std::runtime_error on failure
  void Resume();

  /// Wait for the process to return (blocks the calling thread) and return it's exit code
  void Wait();

  /// Poll the launched process and return true if the process has exited
  bool Poll();

  /// Inject the DLL
  void Inject(std::wstring dll);

  /// Return the exit code or NULL if the process has not yet exited
  const u32* GetExitCode();

  /// Get the process identifier
  u32 GetPid();

  /// Get the main thread identifier
  u32 GetTid();

  /// Get the process handle
  HANDLE GetProcessHandle();

  /// Get the main thread handle
  HANDLE GetThreadHandle();

 private:
  bool TrySetExitCode();
  void OpenProcess(u32 pid);
  void OpenThread();

 private:
  HANDLE m_hProcess = NULL;
  HANDLE m_hThread = NULL;
  std::optional<u32> m_pid;
  std::optional<u32> m_tid;
  std::optional<u32> m_exitCode;
};

/// Kill the process given by `pid`
extern void KillProcess(Context* ctx, DWORD pid);

/// Kill all processes given by `name`
extern void KillProcess(Context* ctx, std::wstring_view name);

}  // namespace bifrost