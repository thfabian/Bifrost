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

#include "bifrost/core/common.h"

#include "bifrost/core/hook_debugger.h"
#include "bifrost/core/hook_settings.h"
#include "bifrost/core/exception.h"
#include "bifrost/core/error.h"
#include "bifrost/core/timer.h"

#pragma warning(push)
#pragma warning(disable : 4091)
#include <DbgHelp.h>
#pragma comment(lib, "dbghelp")
#pragma warning(pop)

namespace bifrost {

namespace {

/// Wrapper for SYMBOL_INFO_PACKAGE structure
struct SymbolInfoPackage : public ::SYMBOL_INFO_PACKAGE {
  SymbolInfoPackage() {
    si.SizeOfStruct = sizeof(::SYMBOL_INFO);
    si.MaxNameLen = sizeof(name);
  }
};

/// Wrapper for IMAGEHLP_LINE64 structure
struct ImageHlpLine64 : public IMAGEHLP_LINE64 {
  ImageHlpLine64() { SizeOfStruct = sizeof(IMAGEHLP_LINE64); }
};

}  // namespace

HookDebugger::HookDebugger(Context* ctx, HookSettings* settings) : Object(ctx), m_settings(settings) {}

HookDebugger::~HookDebugger() {
  if (m_init) {
    BIFROST_CHECK_WIN_CALL(::SymCleanup(::GetCurrentProcess()));
  }
}

const char* HookDebugger::SymbolFromAdress(Context* ctx, const HookTarget& target) {
  if (target.Type == EHookType::E_VTable) {
    void* method = (void*)(*(std::intptr_t*)target.GetTarget());
    return SymbolFromAdress(ctx, method);
  }
  return SymbolFromAdress(ctx, target.GetTarget());
}

const char* HookDebugger::SymbolFromAdress(Context* ctx, void* addr) { return SymbolFromAdress(ctx, (u64)addr); }

const char* HookDebugger::SymbolFromAdress(Context* ctx, u64 addr) {
  // Is this symbol known?
  auto it = m_symbolCache.find(addr);
  if (it != m_symbolCache.end()) return it->second.c_str();

  if (m_symbolResolving && m_init) {
    DWORD64 dwDisplacement = 0;
    DWORD64 dwAddress = addr;
    SymbolInfoPackage info;

    // Resolve jump tables
    bool isJumpTable = false;
    if (auto it = m_jumpTableToTarget.find(dwAddress); it != m_jumpTableToTarget.end()) {
      dwAddress = it->second;
      isJumpTable = true;
    }

    // Resolve trampolines
    bool isTrampoline = false;
    if (auto it = m_trampolineToTarget.find(dwAddress); it != m_trampolineToTarget.end()) {
      dwAddress = it->second;
      isTrampoline = true;
    }

    // Load the symbol from the given address
    std::string symbolName;
    if (::SymFromAddr(::GetCurrentProcess(), dwAddress, &dwDisplacement, &info.si) == TRUE) {
      symbolName = std::string{info.si.Name, info.si.NameLen};

    } else {
      // Failed to load the symbol -> take address
      auto winErr = GetLastWin32Error();
      ctx->Logger().WarnFormat("Failed to get symbol name of 0x%08x: %s", dwAddress, winErr.substr(0, winErr.size() - 1).c_str());
      symbolName = StringFormat("0x%08x", dwAddress);
    }

    if (isJumpTable) symbolName += " [jump-table]";
    if (isTrampoline) symbolName += " [trampoline]";

    return m_symbolCache.emplace(addr, std::move(symbolName)).first->second.c_str();
  } else {
    return m_symbolCache.emplace(addr, StringFormat("0x%08x", addr)).first->second.c_str();
  }
}

void HookDebugger::EnablerOrRefreshSymbolResolving() {
  m_symbolResolving = true;
  if (m_symbolResolving) {
    if (!m_init) {
      DWORD options = ::SymGetOptions();
      if (m_settings->VerboseDbgHelp) options |= SYMOPT_DEBUG;
      options |= SYMOPT_UNDNAME;
      ::SymSetOptions(options);

      BIFROST_CHECK_WIN_CALL(::SymInitialize(::GetCurrentProcess(), NULL, FALSE) == TRUE);

      m_symbolCache.reserve(1024);

      m_trampolineToTarget.reserve(512);
      m_targetToTrampoline.reserve(512);

      m_jumpTableToTarget.reserve(512);

      m_init = true;
    }

    Timer timer;
    Logger().Debug("Loading symbols ...");

    // The documentation of this function is not accurate, the return value is non-zero on success..
    BIFROST_CHECK_WIN_CALL(::SymRefreshModuleList(::GetCurrentProcess()) != FALSE);

    Logger().DebugFormat("Done loading symbols (took %u ms)", timer.Stop());
  }
}

void HookDebugger::RegisterTrampoline(void* trampoline, void* target) {
  if (!m_symbolResolving || !m_init) return;

  m_trampolineToTarget[(u64)trampoline] = (u64)target;
  m_targetToTrampoline[(u64)target] = m_trampolineToTarget.find((u64)trampoline);
}

void HookDebugger::UnregisterTrampoline(void* target) {
  if (!m_symbolResolving || !m_init) return;

  m_trampolineToTarget.erase(m_targetToTrampoline[(u64)target]);
  m_targetToTrampoline.erase((u64)target);
}

void HookDebugger::RegisterJumpTable(void* tableEntryPoint, void* target) {
  if (!m_symbolResolving || !m_init) return;

  m_jumpTableToTarget[(u64)tableEntryPoint] = (u64)target;
}

void HookDebugger::UnregisterJumpTable(void* tableEntryPoint) {
  if (!m_symbolResolving || !m_init) return;

  m_jumpTableToTarget.erase((u64)tableEntryPoint);
}

}  // namespace bifrost
