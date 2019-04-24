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

namespace bifrost {

Process::Process(Context* ctx, LaunchArguments args) : Object(ctx), m_args(std::move(args)) {
  m_startupInfo.cb = sizeof(m_startupInfo);
  ZeroMemory(&m_startupInfo, sizeof(m_startupInfo));
  ZeroMemory(&m_procInfo, sizeof(m_procInfo));

  std::string arguments = "";
  for (int i = 0; i < m_args.Arguments.size(); ++i) arguments += (i == 0 ? "\"" : " \"") + m_args.Arguments[i] + "\"";

  std::wstring cmdStr = m_args.Executable.wstring() + L" " + StringToWString(arguments);
  Logger().InfoFormat(L"Launching process: %s", cmdStr.c_str());

  DWORD creationFlags = 0;
  if (m_args.Suspended) {
    creationFlags |= CREATE_SUSPENDED;
  }

  auto cmd = StringCopy(cmdStr);
  BIFROST_ASSERT_WIN_CALL_MSG(::CreateProcessW(NULL,            // No module name (use command line)
                                               cmd.get(),       // Command line
                                               NULL,            // Process handle not inheritable
                                               NULL,            // Thread handle not inheritable
                                               FALSE,           // Set handle inheritance to FALSE
                                               creationFlags,   // Creation flags
                                               NULL,            // Use parent's environment block
                                               NULL,            // Use parent's starting directory
                                               &m_startupInfo,  // Pointer to STARTUPINFO structure
                                               &m_procInfo) != FALSE,
                              StringFormat("Failed to launch process: %s", WStringToString(cmdStr)).c_str());
}

Process::~Process() {
  BIFROST_CHECK_WIN_CALL(::CloseHandle(m_procInfo.hProcess) != FALSE);
  BIFROST_CHECK_WIN_CALL(::CloseHandle(m_procInfo.hThread) != FALSE);
}

void Process::Resume() { BIFROST_ASSERT_WIN_CALL(::ResumeThread(m_procInfo.hThread) != ((DWORD)-1)); }

void Process::Wait() { BIFROST_ASSERT_WIN_CALL(::WaitForSingleObject(m_procInfo.hProcess, INFINITE) != WAIT_FAILED); }

bool Process::Poll() { return TrySetExitCode(); }

const i32* Process::ExitCode() {
  if (m_exitCode.has_value() || TrySetExitCode()) return &m_exitCode.value();
  return nullptr;
}

bool Process::TrySetExitCode() {
  if (m_exitCode.has_value()) return true;

  DWORD exitCode = 0;
  bool success = false;
  BIFROST_CHECK_WIN_CALL((success = ::GetExitCodeProcess(m_procInfo.hProcess, &exitCode) != FALSE));
  if (success && exitCode != STILL_ACTIVE) {
    m_exitCode = exitCode;
    return true;
  }
  return false;
}

}  // namespace bifrost