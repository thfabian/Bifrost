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

namespace bifrost {

// RAII construction of COM
struct ComInit : public Object {
  ComInit(Context* ctx) : Object(ctx) { BIFROST_ASSERT_COM_CALL(::CoInitialize(NULL)); }
  ~ComInit() { ::CoUninitialize(); }
};

class Debugger::DebuggerImpl : public Object {
 public:
  DebuggerImpl(Context* ctx) : Object(ctx) {}

  void Attach(u32 pid) {
    Logger().InfoFormat("Attaching Visual Studio Debugger to %u ...", pid);
    try {
      ComInit com(&GetContext());

      // Look up CLSID in the registry (VisualStudio.DTE maps to any Visual Studio)
      CLSID clsid;
      BIFROST_ASSERT_COM_CALL(::CLSIDFromProgID(L"VisualStudio.DTE", &clsid));

      // Get the running object which has been registered with OLE
      IUnknown* unknown;
      BIFROST_ASSERT_COM_CALL(::GetActiveObject(clsid, 0, &unknown));

      // Get pointer to the _DTE interface
      CComPtr<EnvDTE::_DTE> dte;
      BIFROST_ASSERT_COM_CALL(unknown->QueryInterface(&dte));

      EnvDTE::Debugger* debugger;
      BIFROST_ASSERT_COM_CALL(dte->get_Debugger(&debugger));

      // Get a list of all processes
      EnvDTE::Processes* procs;
      BIFROST_ASSERT_COM_CALL(debugger->get_LocalProcesses(&procs));

      long numProcs = 0;
      BIFROST_ASSERT_COM_CALL(procs->get_Count(&numProcs));

      EnvDTE::Process* targetProcess = nullptr;

      // Find the target process
      for (long i = 1; i <= numProcs; i++) {
        EnvDTE::Process* proc;
        if (FAILED(procs->Item(variant_t(i), &proc))) continue;

        long procID;
        if (FAILED(proc->get_ProcessID(&procID))) continue;

        if (procID == pid) {
          targetProcess = proc;
          break;
        }
      }

      if (targetProcess) {
        BIFROST_ASSERT_COM_CALL(targetProcess->Attach());
        Logger().Info("Successfully attached Visual Studio Debugger");
      } else {
        throw Exception("Failed to attach Visual Studio Debugger to %u: No process found with given pid", pid);
      }

    } catch (...) {
      Logger().Error("Failed to attach Visual Studio Debugger");
      throw;
    }
  }
};

Debugger::Debugger(Context* ctx) : Object(ctx) { m_impl = std::make_unique<DebuggerImpl>(ctx); }

Debugger::~Debugger() = default;

void Debugger::Attach(u32 pid) { m_impl->Attach(pid); }

}  // namespace bifrost