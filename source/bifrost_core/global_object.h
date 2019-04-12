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
#include "bifrost_core/type.h"
#include "bifrost_core/non_copyable.h"

namespace bifrost {

class ApiShared;
class ApiLoader;
class ModuleLoader;
class Logging;

/// Global object shared among all threads of the DLL
class GlobalObject : NonCopyable {
 public:
  /// Free all global objects
  static void Free();

  /// Get singleton instance
  static GlobalObject& Get();

  /// Deallocate all memory
  ~GlobalObject();

  ApiShared& GetApiShared();
  ApiLoader& GetApiLoader();
  ModuleLoader& GetModuleLoader();
  Logging& GetLogging();

 private:
  static std::mutex s_mutex;
  static GlobalObject* s_instance;

  ApiShared* m_ApiShared = nullptr;
  ApiLoader* m_ApiLoader = nullptr;
  ModuleLoader* m_ModuleLoader = nullptr;
  Logging* m_Logging = nullptr;
};

/// Access the global object
extern GlobalObject& Globals();

}  // namespace bifrost