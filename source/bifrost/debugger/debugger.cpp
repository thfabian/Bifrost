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

#include "bifrost/debugger/common.h"
#include "bifrost/debugger/debugger.h"
#include "bifrost/debugger/error.h"

// https://handmade.network/forums/wip/t/1479-sample_code_to_programmatically_attach_visual_studio_to_a_process

namespace bifrost {

class Debugger::DebuggerImpl : public Object {
 public:
  DebuggerImpl(Context* ctx) : Object(ctx) {
    BIFROST_ASSERT_COM_CALL(::CoInitialize(NULL));

    CLSID Clsid;
    BIFROST_ASSERT_COM_CALL(::CLSIDFromProgID(L"VisualStudio.DTE", &Clsid));
  }

 private:
  bool m_attached;
};

Debugger::Debugger(Context* ctx) : Object(ctx) { m_impl = std::make_unique<DebuggerImpl>(ctx); }

bool Debugger::Attach(u32 pid) { return true; }

}  // namespace bifrost