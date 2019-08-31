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
#include "bifrost/core/exception.h"
#include "bifrost/core/json.h"
#include <tlhelp32.h>

namespace bifrost {
#pragma optimize("", off)

#pragma pack(push)
#pragma pack(1)
struct ThreadParameter {
  // Procedures
  u64 LoadLibraryW_A;
  u64 GetLastError_A;
  u64 GetProcAddress_A;
  u64 CreateThread_A;
  u64 WaitForSingleObject_A;
  u64 GetExitCodeThread_A;
  u64 CloseHandle_A;
  u64 DisableThreadLibraryCalls_A;

  // Data
  u32 DllNameSize;
  u64 DllNamePtr;
  u32 InitProcNameSize;
  u64 InitProcNamePtr;
  u32 InitProcArgSize;
  u64 InitProcArgPtr;
  u32 Timeout;
};
#pragma pack(pop)

enum StageEnum { E_LoadLibraryW = 1, E_GetProcAddress, E_CreateThread, E_WaitForSingleObject, E_Timeout, E_Abandoned, E_GetExitCodeThread, E_Done };

#pragma code_seg(push, ".bf$a")
__declspec(noinline) static DWORD WINAPI Injector(LPVOID lpThreadParameter) {
  using LoadLibraryW_fn = decltype(&::LoadLibraryW);
  using GetLastError_fn = decltype(&::GetLastError);
  using GetProcAddress_fn = decltype(&::GetProcAddress);
  using CreateThread_fn = decltype(&::CreateThread);
  using WaitForSingleObject_fn = decltype(&::WaitForSingleObject);
  using GetExitCodeThread_fn = decltype(&::GetExitCodeThread);
  using CloseHandle_fn = decltype(&::CloseHandle);
  using DisableThreadLibraryCalls_fn = decltype(&::DisableThreadLibraryCalls);

  ThreadParameter* param = (ThreadParameter*)lpThreadParameter;

  // Load the library
  HMODULE hModule = ((LoadLibraryW_fn)param->LoadLibraryW_A)((LPCWSTR)param->DllNamePtr);
  if (hModule == NULL) return (E_LoadLibraryW << 16) + ((GetLastError_fn)param->GetLastError_A)();

  // Disable thread calls
  ((DisableThreadLibraryCalls_fn)param->DisableThreadLibraryCalls_A)(hModule);

  // Create a thread which calls `InitProcName` with `InitProcData` and return the thread id if everything goes as planned
  DWORD threadId = 0;
  if (param->InitProcNameSize == 0) return (E_Done << 16) + NO_ERROR;

  // Get InitProcName
  auto initProc = (DWORD(*)(LPVOID))((GetProcAddress_fn)param->GetProcAddress_A)(hModule, (LPCSTR)param->InitProcNamePtr);
  if (initProc == NULL) return (E_GetProcAddress << 16) + ((GetLastError_fn)param->GetLastError_A)();

  // Launch thread
  HANDLE hThread = ((CreateThread_fn)param->CreateThread_A)(0, 0, initProc, (void*)param->InitProcArgPtr, 0, &threadId);
  if (hThread == NULL) return (E_CreateThread << 16) + ((GetLastError_fn)param->GetLastError_A)();

  // Wait for completion
  DWORD reason = ((WaitForSingleObject_fn)param->WaitForSingleObject_A)(hThread, param->Timeout);
  if (reason == WAIT_FAILED) return (E_WaitForSingleObject << 16) + ((GetLastError_fn)param->GetLastError_A)();
  if (reason == WAIT_TIMEOUT) return (E_Timeout << 16);
  if (reason == WAIT_ABANDONED) return (E_Abandoned << 16);

  // Get the remote thread exit code
  DWORD exitCode;
  if (((GetExitCodeThread_fn)param->GetExitCodeThread_A)(hThread, &exitCode) == FALSE)
    return (E_GetExitCodeThread << 16) + ((GetLastError_fn)param->GetLastError_A)();

  ((CloseHandle_fn)param->CloseHandle_A)(hThread);
  return (E_Done << 16) + exitCode;
}
__declspec(noinline) static DWORD WINAPI InjectorSectionEnd() { return 0; }
#pragma code_seg()
#pragma optimize("", on)

Process::Process(Context* ctx, LaunchArguments args) : Object(ctx) {
  ::STARTUPINFOW startupInfo;
  ZeroMemory(&startupInfo, sizeof(startupInfo));
  startupInfo.cb = sizeof(startupInfo);

  ::PROCESS_INFORMATION procInfo;
  ZeroMemory(&procInfo, sizeof(procInfo));

  std::wstring cmdStr = args.Executable + L" " + StringToWString(args.Arguments);
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
    throw Exception(ss.str());
  } else if (pids.size() == 1) {
    OpenProcess(pids[0]);
  }

  if (m_hProcess == NULL) {
    throw Exception(L"Failed to open process given by name: %s", std::wstring(name).c_str());
  }
  Logger().InfoFormat("Opened process with pid: %u", GetPid());
}

Process::~Process() {
  if (m_hProcess) BIFROST_CHECK_WIN_CALL(::CloseHandle(m_hProcess) != FALSE);
  if (m_hThread) BIFROST_CHECK_WIN_CALL(::CloseHandle(m_hThread) != FALSE);
}

void Process::Suspend() {
  if (Poll()) return;

  Logger().DebugFormat("Suspending thread %u ...", GetTid());
  DWORD suspendCount = 0;
  BIFROST_ASSERT_WIN_CALL((suspendCount = ::SuspendThread(GetThreadHandle())) != ((DWORD)-1));
  Logger().DebugFormat("Successfully suspended thread %u, thread %s (previous suspend count %u)", GetTid(),
                       suspendCount == 0 ? "was running" : "was already suspended", suspendCount);
}

void Process::Resume() {
  if (Poll()) return;

  Logger().DebugFormat("Resuming thread %u ...", GetTid());
  DWORD suspendCount = 0;
  BIFROST_ASSERT_WIN_CALL((suspendCount = ::ResumeThread(GetThreadHandle())) != ((DWORD)-1));
  Logger().DebugFormat("Successfully resumed thread %u, thread is %s (previous suspend count %u)", GetTid(),
                       suspendCount <= 1 ? "now running" : "still suspended", suspendCount);
}

u32 Process::Wait(u32 timeout) {
  u32 reason = 0;
  BIFROST_ASSERT_WIN_CALL((reason = ::WaitForSingleObject(m_hProcess, timeout == 0 ? INFINITE : timeout)) != WAIT_FAILED);
  return reason;
}

bool Process::Poll() { return GetExitCode() != nullptr; }

std::shared_ptr<void> Process::AllocateRemoteMemory(const void* data, u32 sizeInBytes, DWORD protectionFlag, const char* reason) {
  void* ptr = nullptr;
  BIFROST_ASSERT_WIN_CALL((ptr = ::VirtualAllocEx(m_hProcess, NULL, sizeInBytes, MEM_COMMIT, PAGE_EXECUTE_READWRITE)) != NULL);
  std::shared_ptr<void> remoteMem(ptr, [this](void* p) { BIFROST_CHECK_WIN_CALL(::VirtualFreeEx(m_hProcess, p, 0, MEM_RELEASE) != NULL); });

  SIZE_T numBytesWritten = 0;
  BIFROST_ASSERT_WIN_CALL_MSG(::WriteProcessMemory(m_hProcess, remoteMem.get(), data, sizeInBytes, &numBytesWritten) != FALSE,
                              "Failed to write remote process memory");
  if (numBytesWritten != sizeInBytes) {
    throw Exception("Failed to write remote process memory for %s: Requested to write %lu bytes, wrote %lu bytes.", reason, sizeInBytes, numBytesWritten);
  }
  return remoteMem;
}

DWORD Process::RunRemoteThread(const char* functionName, LPTHREAD_START_ROUTINE threadFunc, void* threadParam, u32 timeoutInMs) {
  // Launch the remote thread
  Logger().DebugFormat("Launching remote thread for %s function ...", functionName);
  HANDLE hRemoteThread = NULL;
  BIFROST_ASSERT_WIN_CALL((hRemoteThread = ::CreateRemoteThread(m_hProcess, NULL, 0, threadFunc, threadParam, 0, NULL)) != NULL);

  // Wait for thread to return
  auto timeoutStrInS = timeoutInMs == INFINITE ? std::string("infinitely") : std::to_string(timeoutInMs / 1000);
  Logger().DebugFormat("Waiting %s seconds for %s function to return ...", timeoutStrInS.c_str(), functionName);
  DWORD reason = 0;
  BIFROST_ASSERT_WIN_CALL((reason = ::WaitForSingleObject(hRemoteThread, timeoutInMs)) != WAIT_FAILED);
  if (reason == WAIT_TIMEOUT) {
    throw Exception("Waiting for %s function timed out after %s seconds", functionName, timeoutStrInS.c_str());
  } else if (reason == WAIT_ABANDONED) {
    throw Exception("Waiting for %s function was abandoned", functionName);
  }

  DWORD remoteThreadExitCode = 0;
  BIFROST_ASSERT_WIN_CALL(::GetExitCodeThread(hRemoteThread, &remoteThreadExitCode) != FALSE);
  Logger().DebugFormat("%s function returned - exit code %u", functionName, remoteThreadExitCode & ((~0u) >> 16));

  BIFROST_ASSERT_WIN_CALL(::CloseHandle(hRemoteThread) != FALSE);
  return remoteThreadExitCode;
}

void Process::Inject(InjectArguments args) {
  try {
    Logger().InfoFormat(L"Injecting Dll \"%s\" into remote process %u ...", args.DllPath.c_str(), GetPid());
    if (!std::filesystem::exists(std::filesystem::path(args.DllPath))) {
      throw Exception(L"Dll \"%s\" does not exists", args.DllPath.c_str());
    }

    // Get the kernel32 module
    HMODULE hKernel32 = NULL;
    BIFROST_ASSERT_WIN_CALL((hKernel32 = ::GetModuleHandleW(L"kernel32")) != NULL);

    // Allocate memory for the dll path
    const void* hostDllPtr = args.DllPath.c_str();
    u32 hostDllNameSize = sizeof(wchar_t) * (args.DllPath.size() + 1);
    auto threadDllNamePtr = AllocateRemoteMemory(hostDllPtr, hostDllNameSize, PAGE_READWRITE, "dll path");

    // Allocate memory for init procedure
    std::shared_ptr<void> threadInitProcName;
    u32 hostInitProcNameSize = 0;
    if (!args.InitProcName.empty()) {
      const void* hostInitProcNamePtr = args.InitProcName.c_str();
      hostInitProcNameSize = sizeof(char) * (args.InitProcName.size() + 1);
      threadInitProcName = AllocateRemoteMemory(hostInitProcNamePtr, hostInitProcNameSize, PAGE_READWRITE, "init procedure name");
    }

    // Allocate memory for init procedure
    std::shared_ptr<void> threadInitProcArg;
    u32 hostInitProcArgSize = 0;
    if (!args.InitProcArg.empty()) {
      const void* hostInitProcArgPtr = args.InitProcArg.c_str();
      hostInitProcArgSize = sizeof(char) * (args.InitProcArg.size() + 1);
      threadInitProcArg = AllocateRemoteMemory(hostInitProcArgPtr, hostInitProcArgSize, PAGE_READWRITE, "init procedure argument");
    }

    // Allocate memory for the thread parameter containing the address of the function which are going to be called - This works because kernel32 is always
    // mapped to the same address in each process.
    ThreadParameter param = {0};
    BIFROST_ASSERT_WIN_CALL((param.LoadLibraryW_A = (u64)::GetProcAddress(hKernel32, "LoadLibraryW")) != NULL);
    BIFROST_ASSERT_WIN_CALL((param.GetLastError_A = (u64)::GetProcAddress(hKernel32, "GetLastError")) != NULL);
    BIFROST_ASSERT_WIN_CALL((param.GetProcAddress_A = (u64)::GetProcAddress(hKernel32, "GetProcAddress")) != NULL);
    BIFROST_ASSERT_WIN_CALL((param.CreateThread_A = (u64)::GetProcAddress(hKernel32, "CreateThread")) != NULL);
    BIFROST_ASSERT_WIN_CALL((param.WaitForSingleObject_A = (u64)::GetProcAddress(hKernel32, "WaitForSingleObject")) != NULL);
    BIFROST_ASSERT_WIN_CALL((param.GetExitCodeThread_A = (u64)::GetProcAddress(hKernel32, "GetExitCodeThread")) != NULL);
    BIFROST_ASSERT_WIN_CALL((param.CloseHandle_A = (u64)::GetProcAddress(hKernel32, "CloseHandle")) != NULL);
    BIFROST_ASSERT_WIN_CALL((param.DisableThreadLibraryCalls_A = (u64)::GetProcAddress(hKernel32, "DisableThreadLibraryCalls")) != NULL);

    param.DllNameSize = hostDllNameSize;
    param.DllNamePtr = (u64)threadDllNamePtr.get();
    param.InitProcNameSize = hostInitProcNameSize;
    param.InitProcNamePtr = (u64)threadInitProcName.get();
    param.InitProcArgSize = hostInitProcArgSize;
    param.InitProcArgPtr = (u64)threadInitProcArg.get();
    param.Timeout = args.TimeoutInMs;

    auto threadParam = AllocateRemoteMemory(&param, sizeof(ThreadParameter), PAGE_READWRITE, "thread parameter");

    // Allocate memory for the thread function
    const void* InjectorPtr = &Injector;
    u32 InjectorSize = static_cast<u32>((u64)&InjectorSectionEnd - (u64)&Injector);
    auto threadFunc = AllocateRemoteMemory(InjectorPtr, InjectorSize, PAGE_EXECUTE_READWRITE, "thread function");

    // Run the thread and wait on it
    const char* functionName = "Injector";
    DWORD retCode = RunRemoteThread(functionName, (LPTHREAD_START_ROUTINE)threadFunc.get(), threadParam.get(), args.TimeoutInMs);

    // Extract error code [upper 16 bits -> stage, lower 16 bits -> error code]
    StageEnum stage = static_cast<StageEnum>(retCode >> 16);
    DWORD errorCode = retCode & ((~0u) >> 16);

    auto error = [&](const wchar_t* where, std::wstring msg) {
      if (msg[msg.size() - 1] == '\n') msg[msg.size() - 1] = '\0';
      auto formattedMsg = StringFormat(L"Failed to inject \"%s\": %s function in remote process %s: %s", args.DllPath.c_str(),
                                       StringToWString(functionName).c_str(), where, msg.c_str());
      Logger().Error(formattedMsg.c_str());
      throw Exception(formattedMsg);
    };
    auto winError = [&](const wchar_t* where, DWORD ec) { error(where, StringToWString(GetLastWin32Error(ec)).c_str()); };

    switch (stage) {
      case bifrost::E_LoadLibraryW:
        winError(L"failed in LoadLibraryW", errorCode);
      case bifrost::E_GetProcAddress:
        winError(L"failed in GetProcAddress", errorCode);
      case bifrost::E_CreateThread:
        winError(L"failed in CreateThread", errorCode);
      case bifrost::E_WaitForSingleObject:
        winError(L"failed in WaitForSingleObject", errorCode);
      case bifrost::E_Timeout:
        error(L"timed out",
              StringFormat(L"thread timed out after %u ms", args.TimeoutInMs == INFINITE ? L"infinite" : std::to_wstring(args.TimeoutInMs).c_str()).c_str());
      case bifrost::E_Abandoned:
        error(L"timed out", L"wait was abandoned");
      case bifrost::E_GetExitCodeThread:
        winError(L"failed in GetExitCodeThread", errorCode);
      case bifrost::E_Done:
        if (errorCode != 0) {
          error(StringFormat(L"failed, init procedure '%s' returned", StringToWString(args.InitProcName).c_str()).c_str(), std::to_wstring(errorCode).c_str());
        }
        break;
    }

    Logger().Info(L"Injection successful");
  } catch (...) {
    Logger().Error(L"Injection failed");
    throw;
  }
}

void Process::AttachDebugger() {}

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
      throw Exception("Failed to get main thread-id of process: %i", GetPid());
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

void KillProcess(Context* ctx, u32 pid, bool failOnError) {
  HANDLE hProcess = INVALID_HANDLE_VALUE;
  BIFROST_CHECK_WIN_CALL_CTX(ctx, (hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, pid)) != NULL);

  if (hProcess != NULL) {
    ctx->Logger().InfoFormat("Terminating process: %i", pid);
    BIFROST_CHECK_WIN_CALL_CTX(ctx, ::TerminateProcess(hProcess, 9) != FALSE);
    BIFROST_CHECK_WIN_CALL_CTX(ctx, ::CloseHandle(hProcess) != FALSE);
  } else if (failOnError) {
    throw Exception("Failed to terminate process: %u", pid);
  }
}

void KillProcess(Context* ctx, std::wstring_view name, bool failOnError) {
  HANDLE snapshot;
  BIFROST_ASSERT_WIN_CALL_CTX(ctx, (snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) != NULL);

  PROCESSENTRY32 process;
  ZeroMemory(&process, sizeof(process));
  process.dwSize = sizeof(process);

  if (::Process32FirstW(snapshot, &process)) {
    do {
      if (std::wstring_view(process.szExeFile) == name) {
        KillProcess(ctx, process.th32ProcessID, failOnError);
      }
    } while (Process32Next(snapshot, &process));
  }
  BIFROST_CHECK_WIN_CALL_CTX(ctx, ::CloseHandle(snapshot) != FALSE);
}

}  // namespace bifrost