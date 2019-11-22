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

#include "bifrost/core/hook_manager.h"
#include "bifrost/core/hook_mechanism.h"
#include "bifrost/core/hook_settings.h"
#include "bifrost/core/hook_jump_table.h"
#include "bifrost/core/hook_debugger.h"
#include "bifrost/core/mutex.h"
#include "bifrost/core/timer.h"
#include "bifrost/core/exception.h"
#include "bifrost/core/error.h"
#include "bifrost/core/process.h"

namespace bifrost {

namespace {

#define BIFROST_HOOK_DEBUG(...)             \
  if (GetSettings()->Debug) {               \
    ctx->Logger().DebugFormat(__VA_ARGS__); \
  }

/// RAII construct for suspending all threads (besides the calling thread) of this process
class ThreadSuspender {
 public:
  ThreadSuspender(Process* process) {
    m_process = process;
    m_process->GetOtherThreads(m_suspendedThreads);
    m_process->Suspend(m_suspendedThreads, false);
  }
  ~ThreadSuspender() { m_process->Resume(m_suspendedThreads, false); }

 private:
  std::vector<u32> m_suspendedThreads;
  Process* m_process;
};
#define BIFROST_SUSPEND_OTHER_THREADS() ThreadSuspender BIFROST_CONCAT(__suspender_, __LINE__)(GetProcess())

}  // namespace

class HookManager::Impl {
 public:
  void SetUp(Context* ctx) {
    BIFROST_LOCK_GUARD(m_mutex);
    m_id = 0;
    m_process = std::make_unique<Process>(ctx, ::GetCurrentProcessId());

    // Allocate the settings
    m_settings = std::make_unique<HookSettings>(ctx);
    m_debugger = std::make_unique<HookDebugger>(ctx, m_settings.get());
    if (m_settings->Debug) EnableDebugImpl(ctx);

    // Initialize the hooking mechanisms
    m_hookMechanisms[static_cast<u32>(EHookType::E_CFunction)] = new MinHook(GetSettings(), GetDebugger());
    m_hookMechanisms[static_cast<u32>(EHookType::E_VTable)] = new VTableHook(GetSettings(), GetDebugger());

    ForEachHookType([this, &ctx](EHookType type) { Get(type)->SetUp(ctx); });
  }

  void TearDown(Context* ctx) {
    BIFROST_LOCK_GUARD(m_mutex);

    // Free the hooking mechanisms
    ForEachHookType([this, &ctx](EHookType type) { Get(type)->TearDown(ctx); });
    ForEachHookType([this](EHookType type) { delete Get(type); });

    m_debugger.release();
    m_settings.release();
  }

  void SetHook(Context* ctx, EHookType type, u32 id, u32 priority, void* target, void* detour, void** original) {
    BIFROST_LOCK_GUARD(m_mutex);
    BIFROST_HOOK_DEBUG("Setting %s hook from %s to %s ...", ToString(type), m_debugger->SymbolFromAdress(ctx, target),
                       m_debugger->SymbolFromAdress(ctx, detour));
    Timer timer;

    HookChain* chain = GetHookChain(target);
    if (!chain) {
      // First time we see this target, create the chain
      chain = CreateHookChain(target, this, target, type);
    }

    // Register the hook
    {
      BIFROST_SUSPEND_OTHER_THREADS();
      *original = chain->Insert(ctx, id, priority, detour);
    }

    BIFROST_HOOK_DEBUG("Done setting hook from %s to %s (took %u ms)", m_debugger->SymbolFromAdress(ctx, target), m_debugger->SymbolFromAdress(ctx, detour),
                       timer.Stop());
  }

  void RemoveHook(Context* ctx, EHookType type, u32 id, void* target) {
    BIFROST_LOCK_GUARD(m_mutex);
    BIFROST_HOOK_DEBUG("Removing %s hook %s ...", ToString(type), m_debugger->SymbolFromAdress(ctx, target));
    Timer timer;

    HookChain* chain = GetHookChain(target);
    if (!chain) {
      BIFROST_HOOK_DEBUG("Skipped removing hook from %s: target has no hook");
      return;
    }

    HookChainNode* node = chain->GetById(id);
    if (!node) {
      BIFROST_HOOK_DEBUG("Skipped removing hook from %s: target has no hook for the specified id");
      return;
    }

    // Remove the hook
    {
      BIFROST_SUSPEND_OTHER_THREADS();
      chain->Remove(ctx, node);
    }

    BIFROST_HOOK_DEBUG("Done removing hook from %s (took %u ms)", m_debugger->SymbolFromAdress(ctx, target), timer.Stop());
  }

  void EnableDebug(Context* ctx) {
    BIFROST_LOCK_GUARD(m_mutex);
    EnableDebugImpl(ctx);
  }

  u32 MakeUniqueId() {
    BIFROST_LOCK_GUARD(m_mutex);
    return m_id++;
  }

  /// Access the hook settings
  HookSettings* GetSettings() { return m_settings.get(); }

  /// Access the current process
  Process* GetProcess() { return m_process.get(); }

  /// Get a reference to the debugger
  HookDebugger* GetDebugger() { return m_debugger.get(); }

  /// Access the mechanisms
  IHookMechanism* Get(EHookType type) {
    BIFROST_ASSERT((u32)type >= 0 && (u32)type < (u32)EHookType::E_NumTypes);
    return m_hookMechanisms[static_cast<u32>(type)];
  }

  /// Iterate the mechanisms
  template <class Func>
  void ForEachHookType(Func&& func) {
    for (std::size_t i = (u32)EHookType::E_CFunction; i < (u32)EHookType::E_NumTypes; ++i) func((EHookType)i);
  }

  /// Node of a hook chain
  struct HookChainNode {
    u32 Id;                                    ///< Identifier of the plugin
    u32 Priority;                              ///< Priority of this node
    void* Detour;                              ///< The address of the detour function
    std::unique_ptr<HookJumpTable> JumpTable;  ///< Table which allows to jump to the original function
  };

  /// Chain of hooks
  class HookChain {
   public:
    HookChain(HookManager::Impl* manager, void* target, EHookType type) : m_manager(manager), m_target(target), m_type(type), m_original(nullptr) {
      m_hookChain.reserve(8);
    }

    /// Get the node associated with the `id` or NULL
    HookChainNode* GetById(u32 id) {
      for (auto& chain : m_hookChain) {
        if (chain.Id == id) return &chain;
      }
      return nullptr;
    }

    /// The node is inserted in front of the node which has the same priority. The APP will always call the head of the chain first.
    ///
    /// In order to make the original function available to the hooked functions (aka detours) we create a jump-table and return the address to this table as
    /// the original function. This jump table contains a JMP instruction which jumps to the the next function in the chain. The reason for doing so is that we
    /// can dynamically modify the tables at runtime and thus modify the chain. There is a mode in which only a single hook per target is allowed in which case
    /// we don't have to create a jump table and can immediately return the ORIGINAL function.
    ///
    /// Consider the following example where we have already 2 hooks registered H1 and H2 with priorities 50 and 20, respectively. APP represent the application
    /// and ORIGINAL is the original function. Without any hooks the graph would be "APP ---> ORIGINAL".
    ///
    ///              +----------------+              +----------------+
    ///              |     JMP:H2     | -----+       |  JMP:ORIGINAL  | -----+
    ///              +----------------+      |       +----------------+      |
    ///                      ^               |               ^               |
    ///                      |               |               |               |
    ///              +----------------+      |       +----------------+      |
    ///   APP ---->  |       H1       |      +---->  |       H2       |      +----> ORIGINAL
    ///              |      (50)      |              |      (20)      |
    ///              +----------------+              +----------------+
    ///
    /// We now want to add another hook function, H3, which has priority 20. Meaning, we have to insert it in between H1 and H2 and modify the jump table
    /// of H1. The jump table of the newly inserted H3 will point to H2.
    ///
    ///              +----------------+              +----------------+              +----------------+
    ///              |     JMP:H3     | -----+       |    JMP:H2      | -----+       |  JMP:ORIGINAL  | -----+
    ///              +----------------+      |       +----------------+      |       +----------------+      |
    ///                      ^               |               ^               |               ^               |
    ///                      |               |               |               |               |               |
    ///              +----------------+      |       +----------------+      |       +----------------+      |
    ///   APP ---->  |       H1       |      +---->  |       H3       |      +---->  |       H2       |      +----> ORIGINAL
    ///              |      (50)      |              |      (20)      |              |      (20)      |
    ///              +----------------+              +----------------+              +----------------+
    ///
    /// This function returns the address of the function which should be treated as the original one (the perceived original). For dynamic hooks this will be
    /// entry point of the the jump-table otherwise it's the actual address of the original function.
    void* Insert(Context* ctx, u32 id, u32 priority, void* detour) {
      IHookMechanism* mechanism = m_manager->Get(m_type);

      // Remove any existing hook from this plugin
      HookChainNode* nodeForThisId = GetById(id);
      if (nodeForThisId) Remove(ctx, nodeForThisId);

      // Enforce the condition of "Single" hook strategy
      if (m_manager->GetSettings()->HookStrategy == EHookStrategy::E_Single && Size() > 0) {
        throw Exception("Multiple hooks per target are not allowed with hook strategy '%s'", ToString(EHookStrategy::E_Single));
      }

      std::vector<HookChainNode>::iterator insertedNode;
      HookChainNode newNode{id, priority, detour};

      if (Size() == 0) {
        // This will be the first node in the chain, make sure our detour function is called from the APP ...
        mechanism->SetHook(ctx, m_target, detour, &m_original);

        // ... and we jump to the the ORIGINAL function
        newNode.JumpTable = MakeTable(ctx, detour, m_original);
        insertedNode = m_hookChain.insert(m_hookChain.begin(), std::move(newNode));

      } else {
        // There is already a hook registered. Given the priority we can determine the location where we will be inserted, we have to distinguish 3 cases:
        auto [insertCase, insertIndex] = GetInsertLocation(priority);

        switch (insertCase) {
          // 1) We have a higher or equal priority to head node -> Change the APP hook to our function and our JMP to the detour of the previous head
          case E_First:
            mechanism->RemoveHook(ctx, m_target);
            mechanism->SetHook(ctx, m_target, detour, &m_original);

            newNode.JumpTable = MakeTable(ctx, detour, m_hookChain[0].Detour);

            insertedNode = m_hookChain.insert(m_hookChain.begin(), std::move(newNode));
            break;

          // 2) We have the lowest priority of all nodes -> We will be instered in the back, the previous tail will jump to us and our JMP table will call
          //    ORIGINAL
          case E_Last:
            m_hookChain[m_hookChain.size() - 1].JumpTable->SetTarget(detour);

            newNode.JumpTable = MakeTable(ctx, detour, m_original);

            insertedNode = m_hookChain.insert(m_hookChain.end(), std::move(newNode));
            break;

          // 3) We are in-between two existing nodes -> We have to modify the previous JMP table to call our function and our JMP table has to call the next one
          case E_Middle:
            u32 prevIndex = insertIndex - 1;
            u32 nextIndex = insertIndex;

            m_hookChain[prevIndex].JumpTable->SetTarget(detour);

            newNode.JumpTable = MakeTable(ctx, detour, m_hookChain[nextIndex].Detour);

            insertedNode = m_hookChain.insert(m_hookChain.begin() + insertIndex, std::move(newNode));
            break;
        }
      }

      void* perceivedOriginal = insertedNode->JumpTable ? insertedNode->JumpTable->GetTableEntryPoint() : m_original;
      return perceivedOriginal;
    }

    /// Make a HookJumpTable
    std::unique_ptr<HookJumpTable> MakeTable(Context* ctx, void* detour, void* target) const {
      std::unique_ptr<HookJumpTable> table = nullptr;
      if (m_manager->GetSettings()->HookStrategy != EHookStrategy::E_Single) {
        table = std::make_unique<HookJumpTable>(ctx, m_manager->GetSettings(), m_manager->GetDebugger(), m_manager->Get(EHookType::E_CFunction), detour);
        table->SetTarget(target);
      }
      return table;
    }

    enum EInsertCase {
      E_First,
      E_Last,
      E_Middle,
    };

    /// Get the location where a node with `priority` will be inserted
    std::tuple<EInsertCase, u32> GetInsertLocation(u32 priority) const {
      for (u32 idx = 0; idx < m_hookChain.size(); ++idx) {
        if (priority >= m_hookChain[idx].Priority) {
          if (priority == 0) return std::make_tuple(E_First, 0);
          return std::make_tuple(E_Middle, idx);
        }
      }
      return std::make_tuple(E_Last, m_hookChain.size());
    }

    /// Remove the node and potentially re-hook adjacent node(s)
    void Remove(Context* ctx, const HookChainNode* node) {
      IHookMechanism* mechanism = m_manager->Get(m_type);

      if (Size() == 1) {
        m_hookChain.pop_back();
        mechanism->RemoveHook(ctx, m_target);

      } else {
        auto idx = node - &m_hookChain[0];

        if (idx == 0) {
          // This is the head node, remove it and set the APP hook
          mechanism->RemoveHook(ctx, m_target);
          mechanism->SetHook(ctx, m_target, m_hookChain[idx + 1].Detour, &m_original);

        } else if (idx == m_hookChain.size() - 1) {
          // This is the tail node, make the one node before the current tail the new tail by setting it's jump table to ORIGINAL
          m_hookChain[idx - 1].JumpTable->SetTarget(m_original);
        } else {
          // This is a node in the middle, make the previous jump table call the next function
          m_hookChain[idx - 1].JumpTable->SetTarget(m_hookChain[idx + 1].Detour);
        }

        m_hookChain.erase(m_hookChain.begin() + idx);
      }
    }

    /// Get the number of registered hooks
    u32 Size() const { return m_hookChain.size(); }

   private:
    HookManager::Impl* m_manager = nullptr;

    /// Chain of hooks
    std::vector<HookChainNode> m_hookChain;

    /// Target of the function (address or offset into the VTable)
    void* m_target = nullptr;

    /// Type of hook
    EHookType m_type;

    /// Original function
    void* m_original = nullptr;
  };

  HookChain* GetHookChain(void* target) {
    auto it = m_hookTargetToDesc.find((u64)target);
    return it != m_hookTargetToDesc.end() ? &it->second : nullptr;
  }

  template <class... Args>
  HookChain* CreateHookChain(void* target, Args&&... args) {
    return &m_hookTargetToDesc.emplace((u64)target, HookChain{std::forward<Args>(args)...}).first->second;
  }

  void EnableDebugImpl(Context* ctx) {
    ctx->Logger().DebugFormat("Enabling Hook debugging");
    m_settings->Debug = true;
    m_debugger->SetSymbolResolving(true);
  }

 private:
  /// Global access lock
  SpinMutex m_mutex;

  /// Mapping of the target function to the internal description
  std::unordered_map<u64, HookChain> m_hookTargetToDesc;

  /// Hooking mechanisms (C-Function & VTable)
  std::array<IHookMechanism*, (std::size_t)EHookType::E_NumTypes> m_hookMechanisms;

  /// Reference to the current process
  std::unique_ptr<Process> m_process;

  /// Unique identifier to identify different plugins (each individual plugin can have at most 1 hook per target but multiple plugins can have multiple hooks
  /// per target)
  u32 m_id = 0;

  /// Do we run in debug mode i.e. verbose logging?
  bool m_debugMode = false;

  /// Settings of the hook manager
  std::unique_ptr<HookSettings> m_settings;

  /// Debug functionality
  std::unique_ptr<HookDebugger> m_debugger;
};

HookManager::HookManager() { m_impl = std::make_unique<HookManager::Impl>(); }

HookManager::~HookManager() {}

void HookManager::SetUp(Context* ctx) { m_impl->SetUp(ctx); }

void HookManager::TearDown(Context* ctx) { m_impl->TearDown(ctx); }

u32 HookManager::MakeUniqueId() { return m_impl->MakeUniqueId(); }

void HookManager::SetHook(Context* ctx, EHookType type, u32 id, u32 priority, void* target, void* detour, void** original) {
  m_impl->SetHook(ctx, type, id, priority, target, detour, original);
}

void HookManager::RemoveHook(Context* ctx, EHookType type, u32 id, void* target) { m_impl->RemoveHook(ctx, type, id, target); }

void HookManager::EnableDebug(Context* ctx) { m_impl->EnableDebug(ctx); }

}  // namespace bifrost
