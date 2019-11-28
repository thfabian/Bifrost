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
#include "bifrost/core/hook_settings.h"
#include "bifrost/core/hook_debugger.h"
#include "bifrost/core/hook_context.h"

namespace bifrost {

/// Base class of all hooking objects of Bifrost
class HookObject {
 public:
  HookObject(HookSettings* settings, HookDebugger* debugger) : m_settings(settings), m_debugger(debugger) {}

  /// Get the debugger
  inline HookSettings& Settings() { return *m_settings; }
  inline const HookSettings& Settings() const { return *m_settings; }

  /// Get the debugger
  inline HookDebugger& Debugger() { return *m_debugger; }
  inline const HookDebugger& Debugger() const { return *m_debugger; }

  /// Get the symbol name
  inline const char* Sym(Context* ctx, void* addr) { return m_debugger->SymbolFromAdress(ctx, addr); }
  inline const char* Sym(Context& ctx, void* addr) { return m_debugger->SymbolFromAdress(&ctx, addr); }
  inline const char* Sym(Context* ctx, const HookTarget& target) { return m_debugger->SymbolFromAdress(ctx, target); }
  inline const char* Sym(Context& ctx, const HookTarget& target) { return m_debugger->SymbolFromAdress(&ctx, target); }
  inline const char* Sym(HookContext* ctx, const HookTarget& target) { return m_debugger->SymbolFromAdress(ctx->Context, target); }
  inline const char* Sym(HookContext* ctx, void* addr) { return m_debugger->SymbolFromAdress(ctx->Context, addr); }

 private:
  HookSettings* m_settings;
  HookDebugger* m_debugger;
};

#define BIFROST_HOOK_TRACE(ctx, ...)          \
  if (this->Settings().Debug) {               \
    (ctx)->Logger().TraceFormat(__VA_ARGS__); \
  }

}  // namespace bifrost