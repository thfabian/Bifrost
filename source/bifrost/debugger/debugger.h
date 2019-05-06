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
#include "bifrost/core/object.h"

namespace bifrost {

class Debugger : public Object {
 public:
  Debugger(Context* ctx);
  ~Debugger();

  /// Attach the given process to Visual Studio - throws on error
  void Attach(u32 pid);

 private:
  class DebuggerImpl;
  std::unique_ptr<DebuggerImpl> m_impl;
};

}  // namespace bifrost