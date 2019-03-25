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

#include "bifrost_core/common.h"
#include "bifrost_core/module_loader.h"
#include "bifrost_core/error.h"
#include "bifrost_core/util.h"

namespace bifrost {

std::unique_ptr<ModuleLoader> ModuleLoader::m_instance = nullptr;

ModuleLoader::ModuleLoader() {}

ModuleLoader& ModuleLoader::Get() {
  if (!m_instance) {
    m_instance = std::make_unique<ModuleLoader>();
  }
  return *m_instance;
}

HMODULE ModuleLoader::GetModule(const std::string& moduleName) {
  auto it = m_modules.find(moduleName);
  if (it != m_modules.end()) return it->second;

  HMODULE hmodule = NULL;
  BIFROST_ASSERT_WIN_CALL_MSG((hmodule = ::LoadLibraryA(moduleName.c_str())) != NULL, "Failed to load module: '" + moduleName + "'");
  m_modules.emplace(moduleName, hmodule);
  return hmodule;
}

HMODULE ModuleLoader::GetModule(const std::wstring& moduleName) {
  auto str = WStringToString(moduleName);

  auto it = m_modules.find(str);
  if (it != m_modules.end()) return it->second;

  HMODULE hmodule = NULL;
  BIFROST_ASSERT_WIN_CALL_MSG((hmodule = ::LoadLibraryW(moduleName.c_str())) != NULL, "Failed to load module: '" + str + "'");
  m_modules.emplace(str, hmodule);
  return hmodule;
}

}  // namespace bifrost
