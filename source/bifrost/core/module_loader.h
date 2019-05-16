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

namespace bifrost {

/// Load DLLs (modules) and query the state of the current module
///
/// All methods are thread-safe.
class ModuleLoader : public Object {
 public:
  ModuleLoader(Context* ctx);
  ~ModuleLoader();

  struct ModuleDesc {
    std::wstring Path;          ///< Module path passed to LoadLibrary (name or full path)
    std::wstring DllDirectory;  ///< Custom DLL load directory (if necessary)
  };

  /// Get or load module given by `identifier` or throw std::runtime_error on error
  HMODULE GetOrLoadModule(std::string identifier, ModuleDesc desc);

  /// Get module given by `identifier` or throw std::runtime_error if identifier does not exist
  HMODULE GetModule(std::string identifier);

  /// Get the current module
  HMODULE GetCurrentModule();

  /// Get the name of `module`
  std::wstring GetModuleName(HMODULE module);

  /// Get the path of `module`
  std::wstring GetModulePath(HMODULE module);

  /// Get the name of the current module
  std::wstring GetCurrentModuleName();

  /// Get the path of the current module
  std::wstring GetCurrentModulePath();

 private:
  HMODULE GetModuleImpl(std::string identifier, ModuleDesc* desc);

 private:
  struct Module {
    HMODULE Handle;
    bool Loaded;
  };

  std::mutex m_mutex;
  std::unordered_map<std::string, Module> m_modules;
};

}  // namespace bifrost