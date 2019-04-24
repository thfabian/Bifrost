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

[[nodiscard]] static bool FunctionInThisDll() { return true; }

}  // namespace

ModuleLoader::ModuleLoader(Context* ctx) : Object(ctx) {}

ModuleLoader::~ModuleLoader() {
  for (const auto& m : m_modules) {
    if (m.second.Loaded) {
      Logger().DebugFormat(L"Unloading library '%s'", m.first.c_str());
      BIFROST_CHECK_WIN_CALL(::FreeLibrary(m.second.Handle) != 0);
    }
  }
}

HMODULE ModuleLoader::GetModule(const std::string moduleName, std::string dllDirectory) {
  return GetModule(StringToWString(moduleName), dllDirectory.empty() ? std::wstring{} : StringToWString(dllDirectory));
}

HMODULE ModuleLoader::GetModule(const std::wstring moduleName, std::wstring dllDirectory) {
  BIFROST_LOCK_GUARD(m_mutex);

  auto it = m_modules.find(moduleName);
  if (it != m_modules.end()) return it->second.Handle;

  bool loaded = false;
  HMODULE hmodule = ::GetModuleHandleW(moduleName.c_str());
  if (hmodule == NULL) {
    Logger().DebugFormat(L"Loading library '%s'", moduleName.c_str());

    if (!dllDirectory.empty()) {
      BIFROST_ASSERT_WIN_CALL_MSG(::SetDllDirectoryW(dllDirectory.c_str()) != 0, "Failed to add \"" + WStringToString(dllDirectory) + "\" to the DLL search paths");
    } else {
      BIFROST_ASSERT_WIN_CALL_MSG(::SetDllDirectoryW(NULL) != 0, "Failed to restore DLL search path");
    }

    BIFROST_ASSERT_WIN_CALL_MSG((hmodule = ::LoadLibraryW(moduleName.c_str())) != NULL, "Failed to load module: '" + WStringToString(moduleName) + "'");
    loaded = true;
  }
  m_modules.emplace(std::move(moduleName), Module{hmodule, loaded});
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
  wchar_t path[2 * MAX_PATH];
  BIFROST_ASSERT_WIN_CALL_MSG(::GetModuleFileNameW(module, path, ArraySize(path)) != 0, StringFormat("Failed to get name of %p module", module));
  std::filesystem::path p(path);
  return p.filename().native();
}

std::wstring ModuleLoader::GetCurrentModuleName() { return GetModuleName(GetCurrentModule()); }

}  // namespace bifrost
