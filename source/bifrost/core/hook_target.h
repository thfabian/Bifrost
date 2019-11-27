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

/// Types of hooking approaches
enum class EHookType : u32 { E_CFunction = 0, E_VTable, E_NumTypes };

const char* ToString(EHookType type);

/// Target of a hook
struct HookTarget {
  EHookType Type;  ///< Type of target
  void* Target;    ///< Address of the function
};

}  // namespace bifrost
