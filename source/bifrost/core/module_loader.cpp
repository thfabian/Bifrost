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
#include "bifrost/core/module_loader.h"
#include "bifrost/core/error.h"
#include "bifrost/core/util.h"
#include "bifrost/core/mutex.h"

namespace bifrost {

namespace {

static bool FunctionInThisDll() { return true; }

}  // namespace

ModuleLoader::ModuleLoader(Context* ctx) : Object(ctx) {}

ModuleLoader::~ModuleLoader() {
  for (const auto& m : m_modules) {
    if (m.second.Loaded) {
      Logger().DebugFormat("Unloading library \"%s\"", m.first.c_str());
      BIFROST_CHECK_WIN_CALL(::FreeLibrary(m.second.Handle) != 0);
    }
    BIFROST_CHECK_WIN_CALL(m.second.Handle);
  }
}

HMODULE ModuleLoader::GetOrLoadModule(std::string identifier, ModuleDesc desc) { return GetModuleImpl(identifier, &desc); }

HMODULE ModuleLoader::GetModule(std::string identifier) { return GetModuleImpl(identifier, nullptr); }

HMODULE ModuleLoader::GetModuleImpl(std::string identifier, ModuleDesc* desc) {
  BIFROST_LOCK_GUARD(m_mutex);

  auto it = m_modules.find(identifier);
  if (it != m_modules.end()) return it->second.Handle;

  if (!desc) throw Exception("Module \"%s\" does not exist", identifier.c_str());

  bool loaded = false;
  HMODULE hmodule = ::GetModuleHandleW(desc->Path.c_str());
  if (hmodule == NULL) {
    Logger().DebugFormat(L"Loading library \"%s\"", desc->Path.c_str());

    if (!desc->DllDirectory.empty()) {
      BIFROST_ASSERT_WIN_CALL_MSG(::SetDllDirectoryW(desc->DllDirectory.c_str()) != 0,
                                  "Failed to add \"" + WStringToString(desc->DllDirectory) + "\" to the DLL search paths");
    } else {
      BIFROST_ASSERT_WIN_CALL_MSG(::SetDllDirectoryW(NULL) != 0, "Failed to restore DLL search path");
    }

    BIFROST_ASSERT_WIN_CALL_MSG((hmodule = ::LoadLibraryW(desc->Path.c_str())) != NULL, "Failed to load module: \"" + WStringToString(desc->Path) + "\"");
    loaded = true;
  }

  m_modules.emplace(std::move(identifier), Module{hmodule, !desc->Unload ? false : loaded});
  return hmodule;
}

HMODULE ModuleLoader::GetCurrentModule() {
  HMODULE hmodule;
  BIFROST_ASSERT_WIN_CALL_MSG(
      ::GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)&FunctionInThisDll, &hmodule) != 0,
      "Failed to load the current module");
  return hmodule;
}

std::wstring ModuleLoader::GetModuleName(HMODULE module) {
  std::filesystem::path p(GetModulePath(module));
  return p.filename().native();
}

std::wstring ModuleLoader::GetModulePath(HMODULE module) {
  wchar_t path[2 * MAX_PATH];
  BIFROST_ASSERT_WIN_CALL_MSG(::GetModuleFileNameW(module, path, ArraySize(path)) != 0, StringFormat("Failed to get name of module %p", module));
  return path;
}

std::wstring ModuleLoader::GetCurrentModuleName() { return GetModuleName(GetCurrentModule()); }

std::wstring ModuleLoader::GetCurrentModulePath() { return GetModulePath(GetCurrentModule()); }

}  // namespace bifrost
