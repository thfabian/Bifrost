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
#include "bifrost/core/process.h"
#include "bifrost/core/error.h"
#include "bifrost/core/util.h"
#include <tlhelp32.h>

namespace bifrost {

Process::Process(Context* ctx, LaunchArguments args) : Object(ctx) {
  ::STARTUPINFOW startupInfo;
  ZeroMemory(&startupInfo, sizeof(startupInfo));
  startupInfo.cb = sizeof(startupInfo);

  ::PROCESS_INFORMATION procInfo;
  ZeroMemory(&procInfo, sizeof(procInfo));

  std::string arguments = "";
  for (int i = 0; i < args.Arguments.size(); ++i) arguments += (i == 0 ? "\"" : " \"") + args.Arguments[i] + "\"";

  std::wstring cmdStr = args.Executable.wstring() + L" " + StringToWString(arguments);
  Logger().InfoFormat(L"Launching process: %s", cmdStr.c_str());

  DWORD creationFlags = 0;
  if (args.Suspended) {
    creationFlags |= CREATE_SUSPENDED;
  }

  auto cmd = StringCopy(cmdStr);
  BIFROST_ASSERT_WIN_CALL_MSG(::CreateProcessW(NULL,           // No module name (use command line)
                                               cmd.get(),      // Command line
                                               NULL,           // Process handle not inheritable
                                               NULL,           // Thread handle not inheritable
                                               FALSE,          // Set handle inheritance to FALSE
                                               creationFlags,  // Creation flags
                                               NULL,           // Use parent's environment block
                                               NULL,           // Use parent's starting directory
                                               &startupInfo,   // Pointer to STARTUPINFO structure
                                               &procInfo) != FALSE,
                              StringFormat("Failed to launch process: %s", WStringToString(cmdStr)).c_str());

  m_hProcess = procInfo.hProcess;
  m_hThread = procInfo.hThread;
  m_pid = procInfo.dwProcessId;
  m_tid = procInfo.dwThreadId;
  Logger().InfoFormat("Launched process with pid: %u", GetPid());
}

Process::Process(Context* ctx, u32 pid) : Object(ctx) {
  Logger().InfoFormat("Opening process with pid: %u", pid);
  OpenProcess(pid);
}

Process::Process(Context* ctx, std::wstring_view name) : Object(ctx) {
  Logger().InfoFormat(L"Opening process with name: \"%s\"", name.data());

  HANDLE snapshot;
  BIFROST_ASSERT_WIN_CALL((snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) != NULL);

  PROCESSENTRY32 process;
  ZeroMemory(&process, sizeof(process));
  process.dwSize = sizeof(process);

  std::vector<DWORD> pids;
  if (::Process32FirstW(snapshot, &process)) {
    do {
      if (std::wstring_view(process.szExeFile) == name) {
        pids.emplace_back(process.th32ProcessID);
      }
    } while (Process32Next(snapshot, &process));
  }
  BIFROST_CHECK_WIN_CALL(::CloseHandle(snapshot) != FALSE);

  if (pids.size() > 1) {
    std::stringstream ss;
    ss << "Multiple processes found with name \"" << WStringToString(std::wstring(name)) << "\"";
    for (auto pid : pids) ss << "\n - " << pid;
    throw std::runtime_error(ss.str().c_str());
  } else if (pids.size() == 1) {
    OpenProcess(pids[0]);
  }

  if (m_hProcess == NULL) {
    throw std::runtime_error(WStringToString(StringFormat(L"Failed to open process given by name: %s", std::wstring(name).c_str())).c_str());
  }
  Logger().InfoFormat("Opened process with pid: %u", GetPid());
}

Process::~Process() {
  if (m_hProcess) BIFROST_CHECK_WIN_CALL(::CloseHandle(m_hProcess) != FALSE);
  if (m_hThread) BIFROST_CHECK_WIN_CALL(::CloseHandle(m_hThread) != FALSE);
}

void Process::Suspend() {
  Logger().DebugFormat("Suspending thread %u ...", GetTid());
  DWORD suspendCount = 0;
  BIFROST_ASSERT_WIN_CALL((suspendCount = ::SuspendThread(GetThreadHandle())) != ((DWORD)-1));
  Logger().DebugFormat("Successfully suspended thread %u, thread %s (previous suspend count %u)", GetTid(),
                       suspendCount == 0 ? "was running" : "was already suspended", suspendCount);
}

void Process::Resume() {
  Logger().DebugFormat("Resuming thread %u ...", GetTid());
  DWORD suspendCount = 0;
  BIFROST_ASSERT_WIN_CALL((suspendCount = ::ResumeThread(GetThreadHandle())) != ((DWORD)-1));
  Logger().DebugFormat("Successfully resumed thread %u, thread is %s (previous suspend count %u)", GetTid(),
                       suspendCount == 1 ? "now running" : "still suspended", suspendCount);
}

void Process::Wait() { BIFROST_ASSERT_WIN_CALL(::WaitForSingleObject(m_hProcess, INFINITE) != WAIT_FAILED); }

bool Process::Poll() { return GetExitCode() != nullptr; }

DWORD Process::RunRemoteThread(const char* functionName, const void* argData, u32 argSizeInBytes) {
  Logger().DebugFormat("Running %s in a remote thread for process %u", functionName, GetPid());
  std::shared_ptr<void> remoteThreadParameter = nullptr;

  if (argData) {
    // Allocate remote memory for parameter
    Logger().DebugFormat("Allocating %u bytes in remote process memory for thread parameter", argSizeInBytes);
    auto allocateRemoteMemory = [this](u32 size) {
      void* ptr;
      BIFROST_ASSERT_WIN_CALL((ptr = ::VirtualAllocEx(m_hProcess, NULL, size, MEM_COMMIT, PAGE_READWRITE)) != NULL);
      return std::shared_ptr<void>(ptr, [this](void* ptr) { BIFROST_CHECK_WIN_CALL(::VirtualFreeEx(m_hProcess, ptr, 0, MEM_RELEASE) != NULL); });
    };
    remoteThreadParameter = allocateRemoteMemory(argSizeInBytes);

    // Copy parameter to remote memory
    Logger().Debug("Copying thread parameter to remote process memory");
    SIZE_T numBytesWritten = 0;
    BIFROST_ASSERT_WIN_CALL_MSG(::WriteProcessMemory(m_hProcess, remoteThreadParameter.get(), argData, argSizeInBytes, &numBytesWritten) != FALSE,
                                "Failed to write remote process memory");
    if (numBytesWritten != argSizeInBytes) {
      throw std::runtime_error(
          StringFormat("Failed to write remote process memory: Requested to write %lu bytes, wrote %lu bytes.", argSizeInBytes, numBytesWritten).c_str());
    }
  }

  // Launch the remote thread
  Logger().Debug("Launching remote thread ...");
  HMODULE hKernel32 = NULL;
  BIFROST_ASSERT_WIN_CALL((hKernel32 = ::GetModuleHandleW(L"kernel32")) != NULL);

  LPTHREAD_START_ROUTINE remoteThreadFunc = NULL;
  BIFROST_ASSERT_WIN_CALL((remoteThreadFunc = (LPTHREAD_START_ROUTINE)::GetProcAddress(hKernel32, functionName)) != NULL);

  HANDLE hRemoteThread = NULL;
  BIFROST_ASSERT_WIN_CALL((hRemoteThread = ::CreateRemoteThread(m_hProcess, NULL, 0, remoteThreadFunc, remoteThreadParameter.get(), 0, NULL)) != NULL);

  // Wait for the LoadLibrary to return
  Logger().DebugFormat("Waiting for %s to return ...", functionName);
  BIFROST_ASSERT_WIN_CALL(::WaitForSingleObject(hRemoteThread, INFINITE) != WAIT_FAILED);

  DWORD remoteThreadExitCode = NULL;
  BIFROST_ASSERT_WIN_CALL(::GetExitCodeThread(hRemoteThread, &remoteThreadExitCode) != FALSE);
  BIFROST_ASSERT_WIN_CALL(::CloseHandle(hRemoteThread) != FALSE);
  return remoteThreadExitCode;
}

void Process::Inject(std::wstring dllPath) {
  Suspend();
  try {
    Logger().InfoFormat(L"Injecting Dll \"%s\" into remote process %u ...", dllPath.c_str(), GetPid());
    if (!std::filesystem::exists(std::filesystem::path(dllPath))) {
      throw std::runtime_error(WStringToString(StringFormat(L"Dll \"%s\" does not exists", dllPath.c_str())).c_str());
    }

    const void* hostDllPath = dllPath.c_str();
    u32 hostDllPathSize = sizeof(wchar_t) * (dllPath.size() + 1);
    if (RunRemoteThread("LoadLibraryW", hostDllPath, hostDllPathSize) == NULL) {
      throw std::runtime_error(WStringToString(StringFormat(L"Failed to inject \"%s\": LoadLibraryW in remote process failed.", dllPath.c_str())).c_str());
    }
    Logger().Info(L"Injection successful");
    Resume();
  } catch (...) {
    Logger().Error(L"Injection failed");
    Resume();
    throw;
  }
}

const u32* Process::GetExitCode() {
  if (m_exitCode.has_value() || TrySetExitCode()) return &m_exitCode.value();
  return nullptr;
}

u32 Process::GetPid() {
  if (!m_pid.has_value()) m_pid = ::GetProcessId(m_hProcess);
  return m_pid.value();
}

u32 Process::GetTid() {
  if (!m_tid.has_value()) {
    HANDLE snapshot;
    BIFROST_ASSERT_WIN_CALL((snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0)) != NULL);

    THREADENTRY32 tEntry;
    ZeroMemory(&tEntry, sizeof(tEntry));
    tEntry.dwSize = sizeof(THREADENTRY32);

    if (::Thread32First(snapshot, &tEntry)) {
      do {
        if (tEntry.th32OwnerProcessID == GetPid()) {
          m_tid = tEntry.th32ThreadID;
          break;
        }
      } while (::Thread32Next(snapshot, &tEntry));
    }
    BIFROST_CHECK_WIN_CALL(::CloseHandle(snapshot) != FALSE);

    if (!m_tid.has_value()) {
      throw std::runtime_error(StringFormat("Failed to get main thread-id of process: %i", GetPid()).c_str());
    }
  }
  return m_tid.value();
}

HANDLE Process::GetThreadHandle() {
  if (m_hThread == NULL) OpenThread();
  return m_hThread;
}

bool Process::TrySetExitCode() {
  if (m_exitCode.has_value()) return true;

  DWORD exitCode = 0;
  bool success = false;
  BIFROST_CHECK_WIN_CALL((success = ::GetExitCodeProcess(m_hProcess, &exitCode) != FALSE));
  if (success && exitCode != STILL_ACTIVE) {
    m_exitCode = exitCode;
    return true;
  }
  return false;
}

void Process::OpenProcess(u32 pid) {
  m_pid = pid;
  BIFROST_ASSERT_WIN_CALL_MSG((m_hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_pid.value())) != NULL,
                              StringFormat("Failed to open process with pid: %i", m_pid.value()).c_str());
}

void Process::OpenThread() {
  BIFROST_ASSERT_WIN_CALL_MSG((m_hThread = ::OpenThread(THREAD_ALL_ACCESS, FALSE, GetTid())) != NULL,
                              StringFormat("Failed to open main thread of process with pid: %i", GetPid()).c_str());
}

void KillProcess(Context* ctx, DWORD pid) {
  HANDLE hProcess = INVALID_HANDLE_VALUE;
  BIFROST_CHECK_WIN_CALL_CTX(ctx, (hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, pid)) != NULL);
  if (hProcess != NULL) {
    ctx->Logger().InfoFormat("Terminating process: %i", pid);
    BIFROST_CHECK_WIN_CALL_CTX(ctx, ::TerminateProcess(hProcess, 9) != FALSE);
    BIFROST_CHECK_WIN_CALL_CTX(ctx, ::CloseHandle(hProcess) != FALSE);
  }
}

void KillProcess(Context* ctx, std::wstring_view name) {
  HANDLE snapshot;
  BIFROST_ASSERT_WIN_CALL_CTX(ctx, (snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) != NULL);

  PROCESSENTRY32 process;
  ZeroMemory(&process, sizeof(process));
  process.dwSize = sizeof(process);

  if (::Process32FirstW(snapshot, &process)) {
    do {
      if (std::wstring_view(process.szExeFile) == name) {
        KillProcess(ctx, process.th32ProcessID);
      }
    } while (Process32Next(snapshot, &process));
  }
  BIFROST_CHECK_WIN_CALL_CTX(ctx, ::CloseHandle(snapshot) != FALSE);
}

}  // namespace bifrost