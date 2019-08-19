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
#include "bifrost/core/util.h"

namespace bifrost {

namespace {

// RAII construction of COM
struct ComInit : public Object {
  ComInit(Context* ctx) : Object(ctx) { BIFROST_ASSERT_COM_CALL(::CoInitialize(NULL)); }
  ~ComInit() { ::CoUninitialize(); }
};

}  // namespace

class Debugger::DebuggerImpl : public Object {
 public:
  DebuggerImpl(Context* ctx) : Object(ctx) {}

  void Attach(u32 pid, const wchar_t* solution) {
    Logger().InfoFormat("Attaching Visual Studio Debugger to %u ...", pid);
    try {
      ComInit com(&GetContext());

      // Get pointer to the _DTE interface
      CComPtr<EnvDTE::_DTE> dte = GetDTEInstance(solution);

      CComPtr<EnvDTE::Debugger> debugger;
      BIFROST_ASSERT_COM_CALL(dte->get_Debugger(&debugger));

      // Get a list of all processes
      CComPtr<EnvDTE::Processes> procs;
      BIFROST_ASSERT_COM_CALL(debugger->get_LocalProcesses(&procs));

      long numProcs = 0;
      BIFROST_ASSERT_COM_CALL(procs->get_Count(&numProcs));

      CComPtr<EnvDTE::Process> targetProcess = nullptr;

      // Find the target process
      for (long i = 1; i <= numProcs; i++) {
        CComPtr<EnvDTE::Process> proc;
        if (FAILED(procs->Item(variant_t(i), &proc))) continue;

        long procID;
        if (FAILED(proc->get_ProcessID(&procID))) continue;

        if (procID == pid) {
          targetProcess = proc;
          break;
        }
      }

      if (targetProcess) {
        auto solutionName = GetSolutionName(dte);
        if (!solutionName.empty()) Logger().InfoFormat(L"Using Visual Studio with solution \"%s\"", solutionName.c_str());

        Logger().Info("If target process is suspended, the debugger will break at ntdll.dll!LdrpDoDebuggerBreak - you can safely continue");
        BIFROST_ASSERT_COM_CALL(targetProcess->Attach());

        // Sleep a little to avoid "Call was rejected by callee" (Sleep can dead-lock here!)
        MsgWaitForMultipleObjects(0, NULL, false, 5000, QS_ALLINPUT);

        // Focus the window
        CComPtr<EnvDTE::Window> window;
        BIFROST_ASSERT_COM_CALL(dte->get_MainWindow(&window));

        if (window) {
          HWND hwnd;
          BIFROST_ASSERT_COM_CALL(window->get_HWnd((long*)&hwnd));

          BIFROST_ASSERT_WIN_CALL(::ShowWindow(hwnd, SW_SHOWMAXIMIZED));
          BIFROST_ASSERT_WIN_CALL(::SetForegroundWindow(hwnd));
        }
        Logger().Info("Successfully attached Visual Studio Debugger");
      } else {
        throw Exception("Failed to attach Visual Studio Debugger to %u: No process found with given pid", pid);
      }

    } catch (...) {
      Logger().Error("Failed to attach Visual Studio Debugger");
      throw;
    }
  }

 private:
  /// Get the name of the associated solution
  std::wstring GetSolutionName(const CComPtr<EnvDTE::_DTE>& dte) {
    CComPtr<EnvDTE::_Solution> sol;
    BIFROST_ASSERT_COM_CALL(dte->get_Solution(&sol));
    if (sol) {
      wchar_t* solutionPath;
      BIFROST_ASSERT_COM_CALL(sol->get_FileName(&solutionPath));
      return std::filesystem::path(solutionPath).filename().native();
    }

    return L"";
  }

  /// Get the DTE instance associated with `solution` (or any other instance if `solution` is NULL)
  CComPtr<EnvDTE::_DTE> GetDTEInstance(const wchar_t* solution) {
    const std::wstring vsPrefix = L"!VisualStudio";
    IUnknown* instance = nullptr;

    if (solution) {
      std::wstring solutionStr{solution};

      CComPtr<IMalloc> mem;
      BIFROST_ASSERT_COM_CALL(::CoGetMalloc(1, &mem));

      // The running table
      CComPtr<IRunningObjectTable> rot;
      BIFROST_ASSERT_COM_CALL(::GetRunningObjectTable(0, &rot));

      // Iterate the table
      CComPtr<IEnumMoniker> enumMoniker;
      BIFROST_ASSERT_COM_CALL(rot->EnumRunning(&enumMoniker));

      IMoniker* moniker;
      ULONG ret;
      std::vector<CComPtr<EnvDTE::_DTE>> dtes;
      std::wstringstream ssSoltutioNames;

      while (enumMoniker->Next(1, &moniker, &ret) == 0) {
        CComPtr<IBindCtx> bindObj;
        BIFROST_ASSERT_COM_CALL(::CreateBindCtx(0, &bindObj));

        if (bindObj == nullptr) continue;

        // Get the display name and the object itself
        wchar_t* p;
        BIFROST_ASSERT_COM_CALL(moniker->GetDisplayName(bindObj, NULL, &p));
        std::shared_ptr<wchar_t> displayName(p, [&mem](wchar_t* p) { mem->Free(p); });

        // Check if the instance is a VS instance and has our solution open
        if (std::wstring_view(displayName.get()).substr(0, vsPrefix.size()) == vsPrefix) {
          BIFROST_ASSERT_COM_CALL(rot->GetObjectW(moniker, &instance));

          CComPtr<EnvDTE::_DTE> dte;
          BIFROST_ASSERT_COM_CALL(instance->QueryInterface(&dte));

          if (dte) {
            auto solutionName = GetSolutionName(dte);

            // Precise match
            if (StringCompareCaseInsensitive(solutionName, solutionStr)) {
              return dte;
            }

            // Partial match
            if (StringCompareCaseInsensitive(std::wstring_view(solutionName).substr(0, solutionStr.size()), solutionStr)) {
              dtes.emplace_back(dte);
            }

            ssSoltutioNames << L"  " << solutionName << L"\n";
          }
        }
      }

      // Use partial match
      if (dtes.size() == 1) {
        return dtes[0];
      }

      throw Exception(L"No Visual Studio instance with open solution \"%s\" could be found. Solutions open:\n%s", solution, ssSoltutioNames.str().c_str());
      return nullptr;

    } else {
      // Look up CLSID in the registry (VisualStudio.DTE maps to any Visual Studio)
      CLSID clsid;
      BIFROST_ASSERT_COM_CALL(::CLSIDFromProgID(L"VisualStudio.DTE", &clsid));

      // Get the running object which has been registered with OLE (this takes the first registered one)
      BIFROST_ASSERT_COM_CALL_MSG(::GetActiveObject(clsid, 0, &instance), "Query active Visual Studio instances - at least one instance is required");

      CComPtr<EnvDTE::_DTE> dte;
      BIFROST_ASSERT_COM_CALL(instance->QueryInterface(&dte));
      return dte;
    }
  }
};

Debugger::Debugger(Context* ctx) : Object(ctx) { m_impl = std::make_unique<DebuggerImpl>(ctx); }

Debugger::~Debugger() = default;

void Debugger::Attach(u32 pid, const wchar_t* solution) { m_impl->Attach(pid, solution); }

}  // namespace bifrost