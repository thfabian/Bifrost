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
    std::wstring Executable;  ///< Path to the executable
    std::string Arguments;    ///< Arguments passed to the executable;
    bool Suspended = false;   ///< Launch suspended?
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

  /// Get the ids of all threads of this process (clears `threadIds` first)
  void GetThreads(std::vector<u32>& threadIds);
  std::vector<u32> GetThreads();

  /// Get the ids of all threads but the currently executing one of this process (clears `threadIds` first)
  void GetOtherThreads(std::vector<u32>& threadIds);

  /// Suspend the threads - throws std::runtime_error on failure
  void Suspend(const std::vector<u32>& threads, bool verbose = true);

  /// Resume the threads after suspension - throws std::runtime_error on failure
  void Resume(const std::vector<u32>& threads, bool verbose = true);

  /// Wait for the process to return (blocks the calling thread) returns the return value of `WaitForSingleObject`
  u32 Wait(u32 timeout = INFINITE);

  /// Poll the launched process and return true if the process has exited
  bool Poll();

  struct InjectArguments {
    std::wstring DllPath;      ///< Full path to the DLL which is going to be injected
    std::string InitProcName;  ///< Name of the procedure to call in the injected DLL (used as argument to GetProcAddress)
    std::string InitProcArg;   ///< Argument to `InitProcName` procedure
    u32 TimeoutInMs = 5000;    ///< Max time allocated for the injection process
  };

  /// Inject the DLL - throws std::runtime_error on failure
  void Inject(InjectArguments args);

  /// Return the exit code or NULL if the process has not yet exited
  const u32* GetExitCode();

  /// Get the process identifier
  u32 GetPid();

 private:
  bool TrySetExitCode();
  void OpenProcess(u32 pid);
  DWORD RunRemoteThread(const char* functionName, LPTHREAD_START_ROUTINE threadFunc, void* threadParam, u32 timeoutInMs);
  std::shared_ptr<void> AllocateRemoteMemory(const void* data, u32 sizeInBytes, DWORD protectionFlag, const char* reason);

 private:
  HANDLE m_hProcess = NULL;
  HANDLE m_hThread = NULL;
  std::optional<u32> m_pid;
  std::optional<u32> m_tid;
  std::optional<u32> m_exitCode;
  std::vector<u32> m_frozenThreads;
};

/// Kill the process given by `pid`
extern void KillProcess(Context* ctx, u32 pid, bool failOnError = false);

/// Kill all processes given by `name`
extern void KillProcess(Context* ctx, std::wstring_view name, bool failOnError = false);

}  // namespace bifrost