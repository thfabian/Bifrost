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
#include "bifrost/core/context.h"

namespace bifrost {

/// Parameters passed to injector DLL
struct InjectorParam {
  std::string SharedMemoryName;   ///< Name of the shared memory
  u32 SharedMemorySize;           ///< Size of the shared memory
  u32 Pid;                        ///< Identifier of the injector
  std::wstring WorkingDirectory;  ///< Working directory of the injector
  std::string CustomArgument;     ///< Custom arguments passed to the Injector function

  /// Serialize the parameters to a JSON string
  std::string Serialize() const;

  /// Deserialize the parameters from a JSON string - throws on error
  static InjectorParam Deserialize(Context* ctx, const char* jStr);
};

}  // namespace bifrost
