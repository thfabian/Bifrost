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

#include "bifrost/core/hook_target.h"

namespace bifrost {

const char* ToString(EHookType type) {
  switch (type) {
    case EHookType::E_CFunction:
      return "function";
    case EHookType::E_VTable:
      return "vtable";
  };
  return "unknown";
}

}  // namespace bifrost