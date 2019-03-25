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

#include "bifrost_injector/common.h"
#include "bifrost_injector/process.h"
#include "bifrost_core/error.h"
#include "bifrost_core/util.h"
#include "bifrost_core/logging.h"

namespace bifrost::injector {

Process::~Process() {
  BIFROST_CHECK_WIN_CALL(::CloseHandle(m_procInfo.hProcess) != FALSE);
  BIFROST_CHECK_WIN_CALL(::CloseHandle(m_procInfo.hThread) != FALSE);
}

int Process::Wait() {
  BIFROST_CHECK_WIN_CALL(::WaitForSingleObject(m_procInfo.hProcess, INFINITE) != WAIT_FAILED);

  DWORD exitCode = 0;
  BIFROST_CHECK_WIN_CALL(::GetExitCodeProcess(m_procInfo.hProcess, &exitCode));
  return exitCode;
}

std::unique_ptr<Process> Process::Launch(const std::filesystem::path& executable, const std::vector<std::string>& args) {
  ::STARTUPINFO si;
  si.cb = sizeof(si);
  ZeroMemory(&si, sizeof(si));

  ::PROCESS_INFORMATION pi;
  ZeroMemory(&pi, sizeof(pi));

  auto cmdStr = executable.wstring();
  for (const auto& arg : args) cmdStr += L" " + bifrost::StringToWString(arg);

  BIFROST_LOG_INFO(L"Launching process: \"%s\"", cmdStr.c_str());

  auto cmd = bifrost::StringCopy(cmdStr);
  BIFROST_ASSERT_WIN_CALL_MSG(::CreateProcess(NULL,       // No module name (use command line)
                                              cmd.get(),  // Command line
                                              NULL,       // Process handle not inheritable
                                              NULL,       // Thread handle not inheritable
                                              FALSE,      // Set handle inheritance to FALSE
                                              0,          // No creation flags
                                              NULL,       // Use parent's environment block
                                              NULL,       // Use parent's starting directory
                                              &si,        // Pointer to STARTUPINFO structure
                                              &pi) != FALSE,
                              bifrost::StringFormat("Failed to launch process: \"%s\"", bifrost::WStringToString(cmdStr)).c_str());

  return std::make_unique<Process>(std::move(si), std::move(pi));
}

Process::Process(::STARTUPINFO&& startupInfo, ::PROCESS_INFORMATION&& procInfo) : m_startupInfo(std::move(startupInfo)), m_procInfo(std::move(procInfo)) {}

}  // namespace bifrost::injector
