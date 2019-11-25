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
#include "bifrost/core/type.h"

namespace bifrost {

/// Types of hooking approaches
enum class EHookType : u32 { E_CFunction = 0, E_VTable, E_NumTypes };

const char* ToString(EHookType type);

/// Target of a hook
struct HookTarget {
  struct CFunctionT {
    void* Target;  ///< Address of the function
  };

  struct VTableT {
    void* Table;  ///< VTable pointer
    u64 Offset;   ///< Offset, in bytes, to the target method
  };

  /// Type of target
  EHookType Type;

  /// Target function address or offset of the method in the VTable
  union {
    VTableT VTable;
    CFunctionT CFunction;
  };

  /// Get the address of the function or methid
  inline void* GetTarget() const noexcept { return Type == EHookType::E_CFunction ? CFunction.Target : ((std::uint8_t*)VTable.Table + VTable.Offset); }
};

}  // namespace bifrost
