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

#include "bifrost_core/common.h"

namespace bifrost {

/// Load DLLs (modules) and query the state of the current module
class ModuleLoader {
 public:
  ModuleLoader();
  ~ModuleLoader();

  /// Get singleton instance
  static ModuleLoader& Get();

  /// Get module given by ``moduleName`` or throw std::runtime_error on error
  HMODULE GetModule(const std::string& moduleName);
  HMODULE GetModule(const std::wstring& moduleName);
  
  /// Get the current module
  HMODULE GetCurrentModule();

  /// Get the name of ``module``
  std::string GetModuleName(HMODULE module);

  /// Get the name of the current module
  std::string GetCurrentModuleName();

 private:
  struct Module {
    HMODULE Handle;
    bool Loaded;
  };
  static std::unique_ptr<ModuleLoader> m_instance;
  std::unordered_map<std::string, Module> m_modules;
};

}  // namespace bifrost