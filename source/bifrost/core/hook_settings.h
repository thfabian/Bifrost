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
#include "bifrost/core/type.h"

namespace bifrost {

class Context;

/// Types of hooking approaches
enum class EHookType : u32 { E_CFunction = 0, E_VTable, E_NumTypes };

const char* ToString(EHookType type);

/// Strategy used when creating the hook chains
enum class EHookStrategy : u32 {
  E_Multi = 0,  ///< Allow multiple hook's per target and runtime rehooking
  E_Single,     ///< Allow only a single hook per target is supported
};

const char* ToString(EHookStrategy type);

/// Settings to configure the hooking mechanisms via file and environment variables
class HookSettings {
 public:
  HookSettings(Context* ctx);

  /// Debug mode enabled? ENV: BIFROST_HOOK_DEBUG
  bool Debug = false;

  /// Run DbgHelp in verbose mode? ENV: BIFROST_HOOK_VERBOSE_DBGHELP
  bool VerboseDbgHelp = false;

  /// Strategy used when applying hooks ENV: BIFROST_HOOK_STRATEGY
  EHookStrategy HookStrategy = EHookStrategy::E_Multi;
};

}  // namespace bifrost