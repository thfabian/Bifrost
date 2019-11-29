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

#include "hook_plugin.h"

#define CAT(A, B) CAT_(A, B)
#define CAT_(A, B) A##B

#define STR(s) STR_(s)
#define STR_(s) #s

// This plugin is compiled three times so we can test the chain hooking
#ifndef PLUGIN_INDEX
#define PLUGIN_INDEX 1
#endif

#ifndef PLUGIN_NAME
#define PLUGIN_NAME CAT(HookTestPlugin, PLUGIN_INDEX)
#endif

// Make sure we can distinguish the function of the three plugins
#if PLUGIN_INDEX == 1
#define PLUGIN_PREFIX(x) x
#else
#define PLUGIN_PREFIX(x) CAT(CAT(CAT(p, PLUGIN_INDEX), _), x)
#endif

#define BIFROST_NAMESPACE hook_plugin

#define BIFROST_PLUGIN_IDENTIFIER bifrost_add, bifrost_IAdder_add,
#define BIFROST_PLUGIN_IDENTIFIER_TO_STRING "bifrost_add", "bifrost_IAdder_add",
#define BIFROST_PLUGIN_IDENTIFIER_TO_FUNCTION_NAME "bifrost_add", "IAdder::add",
#define BIFROST_PLUGIN_IDENTIFIER_TO_HOOK_TYPE HookType::CFunction, HookType::VTable,
#define BIFROST_PLUGIN_STRING_TO_IDENTIFIER \
  {"bifrost_add", Plugin::Identifier::bifrost_add}, { "bifrost_Adder_add", Plugin::Identifier::bifrost_IAdder_add }
#define BIFROST_PLUGIN_IDENTIFIER_TO_VTABLE_OFFSET -1, 0,

#define BIFROST_PLUGIN_OBJECT_TYPE IAdder,
#define BIFROST_PLUGIN_OBJECT_TYPE_TO_STRING "IAdder",
#define BIFROST_PLUGIN_IDENTIFIER_TO_OBJECT_TYPE Plugin::ObjectType::__invalid__, Plugin::ObjectType::IAdder,

#define BIFROST_PLUGIN_MODULE test_bifrost_api_hook_dll_dll,
#define BIFROST_PLUGIN_MODULE_TO_STRING L"test-bifrost-api-hook-dll.dll",
#define BIFROST_PLUGIN_IDENTIFIER_TO_MODULE Module::test_bifrost_api_hook_dll_dll, Module::test_bifrost_api_hook_dll_dll,

#define BIFROST_PLUGIN_INCLUDES
#include "hook_dll.h"

#define BIFROST_DEBUG 1
#define BIFROST_IMPLEMENTATION
#define BIFROST_PLUGIN_DSL_DEF

#include "bifrost/template/plugin_main.h"

//
// Generated DSL
//

// bifrost_add
#define _bf_func_decl_ret_hook_plugin__bifrost_add int
#define _bf_func_decl_args_hook_plugin__bifrost_add int arg1, int arg2
#define _bf_func_hook_plugin__bifrost_add \
  ((int (*)(int, int))BIFROST_NAMESPACE## ::Plugin::Get().GetHook<BIFROST_NAMESPACE## ::Plugin::Identifier::bifrost_add>()->GetOriginal())

#define _bf_args_hook_plugin__bifrost_add arg1, arg2
#define _bf_arg_1_hook_plugin__bifrost_add arg1
#define _bf_arg_2_hook_plugin__bifrost_add arg2

// bifrost::Adder::add
#define _bf_func_decl_ret_hook_plugin__bifrost_IAdder_add int
#define _bf_func_decl_args_hook_plugin__bifrost_IAdder_add bifrost::IAdder *__this__, int arg1, int arg2
#define _bf_func_hook_plugin__bifrost_IAdder_add                                \
  ((int (*)(bifrost::IAdder*, int, int))BIFROST_NAMESPACE## ::Plugin::Get()     \
       .GetHook<BIFROST_NAMESPACE## ::Plugin::Identifier::bifrost_IAdder_add>() \
       ->GetOriginal())

#define _bf_this_hook_plugin__bifrost_IAdder_add __this__
#define _bf_args_hook_plugin__bifrost_IAdder_add __this__, arg1, arg2
#define _bf_arg_1_hook_plugin__bifrost_IAdder_add arg1
#define _bf_arg_2_hook_plugin__bifrost_IAdder_add arg2

#include "bifrost/api/test/data/shared.h"

using namespace bifrost;

//
// bifrost_add
//
#define bf_id bifrost_add

bf_override(PLUGIN_PREFIX(bifrost_add__original_1)) { return bf_original(bf_args); }
bf_override(PLUGIN_PREFIX(bifrost_add__original_2)) { return bf_original(bf_arg(1), bf_arg(2)); }
bf_override(PLUGIN_PREFIX(bifrost_add__original_3)) { return bf_original(bf_arg(1, 2)); }

bf_override(PLUGIN_PREFIX(bifrost_add__modify_1)) { return bf_original(5, bf_arg(2)); }
bf_override(PLUGIN_PREFIX(bifrost_add__modify_2)) { return bf_original(bf_arg(1), 5); }
bf_override(PLUGIN_PREFIX(bifrost_add__modify_3)) { return bf_original(5, 5); }

bf_override(PLUGIN_PREFIX(bifrost_add__times_2)) { return 2 * bf_original(bf_args); }
bf_override(PLUGIN_PREFIX(bifrost_add__plus_3)) { return 3 + bf_original(bf_args); }

#undef bf_id

//
// bifrost_Adder_add
//
#define bf_id bifrost_IAdder_add

bf_override(PLUGIN_PREFIX(bifrost_IAdder_add__original_1)) { return bf_original(bf_args); }
bf_override(PLUGIN_PREFIX(bifrost_IAdder_add__original_2)) { return bf_original(bf_this, bf_arg(1), bf_arg(2)); }
bf_override(PLUGIN_PREFIX(bifrost_IAdder_add__original_3)) { return bf_original(bf_this, bf_arg(1, 2)); }

bf_override(PLUGIN_PREFIX(bifrost_IAdder_add__modify_1)) { return bf_original(bf_this, 5, bf_arg(2)); }
bf_override(PLUGIN_PREFIX(bifrost_IAdder_add__modify_2)) { return bf_original(bf_this, bf_arg(1), 5); }
bf_override(PLUGIN_PREFIX(bifrost_IAdder_add__modify_3)) { return bf_original(bf_this, 5, 5); }

bf_override(PLUGIN_PREFIX(bifrost_IAdder_add__times_2)) { return 2 * bf_original(bf_args); }
bf_override(PLUGIN_PREFIX(bifrost_IAdder_add__plus_3)) { return 3 + bf_original(bf_args); }

#undef bf_id

class PLUGIN_NAME final : public hook_plugin::Plugin {
 public:
  // Get the file <mode> from "<file>;<mode>"
  std::string GetFile() {
    std::string args(GetArguments());
    return args.substr(0, args.find(";"));
  }

  // Get the file <mode> from "<file>;<mode>"
  Mode GetMode() {
    std::string args(GetArguments());
    auto s = std::atoi(args.substr(args.find(";") + 1).c_str());
    return (Mode)s;
  }

  virtual void SetUp() override {
    switch (GetMode()) {
      //
      // C-Function Single
      //
      case Mode::CFunction_Single_Original1:
        SetHook(Identifier::bifrost_add, PLUGIN_PREFIX(bifrost_add__original_1));
        break;

      case Mode::CFunction_Single_Original2:
        SetHook(Identifier::bifrost_add, PLUGIN_PREFIX(bifrost_add__original_2));
        break;

      case Mode::CFunction_Single_Original3:
        SetHook(Identifier::bifrost_add, PLUGIN_PREFIX(bifrost_add__original_3));
        break;

      case Mode::CFunction_Single_Modify1:
        SetHook(Identifier::bifrost_add, PLUGIN_PREFIX(bifrost_add__modify_1));
        break;

      case Mode::CFunction_Single_Modify2:
        SetHook(Identifier::bifrost_add, PLUGIN_PREFIX(bifrost_add__modify_2));
        break;

      case Mode::CFunction_Single_Modify3:
        SetHook(Identifier::bifrost_add, PLUGIN_PREFIX(bifrost_add__modify_3));
        break;

      case Mode::CFunction_Single_Replace1:
        SetHook(Identifier::bifrost_add, PLUGIN_PREFIX(bifrost_add__modify_3));
        SetHook(Identifier::bifrost_add, PLUGIN_PREFIX(bifrost_add__original_1));
        break;

      case Mode::CFunction_Single_Restore1:
        SetHook(Identifier::bifrost_add, PLUGIN_PREFIX(bifrost_add__modify_1));
        RemoveHook(Identifier::bifrost_add);
        break;

      //
      // C-Function Multi
      //
      case Mode::CFunction_Multi_Original_P1:
      case Mode::CFunction_Multi_Original_P2:
      case Mode::CFunction_Multi_Original_P3:
        SetHook(Identifier::bifrost_add, PLUGIN_PREFIX(bifrost_add__original_1));
        break;

      case Mode::CFunction_Multi_Modify1_P1:
        SetHook(Identifier::bifrost_add, PLUGIN_PREFIX(bifrost_add__times_2));
        break;
      case Mode::CFunction_Multi_Modify1_P2:
        SetHook(Identifier::bifrost_add, PLUGIN_PREFIX(bifrost_add__plus_3));
        break;

      case Mode::CFunction_Multi_Modify2_P1:
        SetHook(Identifier::bifrost_add, PLUGIN_PREFIX(bifrost_add__times_2), 50);
        break;
      case Mode::CFunction_Multi_Modify2_P2:
        SetHook(Identifier::bifrost_add, PLUGIN_PREFIX(bifrost_add__plus_3), 100);
        break;

      case Mode::CFunction_Multi_Modify3_P1:
        SetHook(Identifier::bifrost_add, PLUGIN_PREFIX(bifrost_add__times_2), 100);
        break;
      case Mode::CFunction_Multi_Modify3_P2:
        SetHook(Identifier::bifrost_add, PLUGIN_PREFIX(bifrost_add__plus_3), 50);
        break;

      case Mode::CFunction_Multi_Modify4_P1:
        SetHook(Identifier::bifrost_add, PLUGIN_PREFIX(bifrost_add__times_2));
        break;
      case Mode::CFunction_Multi_Modify4_P2:
        SetHook(Identifier::bifrost_add, PLUGIN_PREFIX(bifrost_add__plus_3));
        break;
      case Mode::CFunction_Multi_Modify4_P3:
        SetHook(Identifier::bifrost_add, PLUGIN_PREFIX(bifrost_add__times_2));
        break;

      case Mode::CFunction_Multi_Modify5_P1:
        SetHook(Identifier::bifrost_add, PLUGIN_PREFIX(bifrost_add__plus_3), 100);
        break;
      case Mode::CFunction_Multi_Modify5_P2:
        SetHook(Identifier::bifrost_add, PLUGIN_PREFIX(bifrost_add__plus_3), 50);
        break;
      case Mode::CFunction_Multi_Modify5_P3:
        SetHook(Identifier::bifrost_add, PLUGIN_PREFIX(bifrost_add__times_2), 50);
        break;

      //
      // VTable Single
      //
      case Mode::VTable_Single_Original1: {
        IAdder* adder = new Adder();
        SetVTableHook(Identifier::bifrost_IAdder_add, adder, PLUGIN_PREFIX(bifrost_IAdder_add__original_1));
        delete adder;
        break;
      }

      case Mode::VTable_Single_Original2: {
        IAdder* adder = new Adder();
        SetVTableHook(Identifier::bifrost_IAdder_add, adder, PLUGIN_PREFIX(bifrost_IAdder_add__original_2));
        delete adder;
        break;
      }

      case Mode::VTable_Single_Original3: {
        IAdder* adder = new Adder();
        SetVTableHook(Identifier::bifrost_IAdder_add, adder, PLUGIN_PREFIX(bifrost_IAdder_add__original_3));
        delete adder;
        break;
      }

      case Mode::VTable_Single_Original4: {
        IAdder* adder = new Adder();
        RegisterVTable(ObjectType::IAdder, adder);
        delete adder;

        SetHook(Identifier::bifrost_IAdder_add, PLUGIN_PREFIX(bifrost_IAdder_add__original_3));
        break;
      }

      case Mode::VTable_Single_Modify1: {
        IAdder* adder = new Adder();
        SetVTableHook(Identifier::bifrost_IAdder_add, adder, PLUGIN_PREFIX(bifrost_IAdder_add__modify_1));
        delete adder;
        break;
      }

      case Mode::VTable_Single_Modify2: {
        IAdder* adder = new Adder();
        SetVTableHook(Identifier::bifrost_IAdder_add, adder, PLUGIN_PREFIX(bifrost_IAdder_add__modify_2));
        delete adder;
        break;
      }

      case Mode::VTable_Single_Modify3: {
        IAdder* adder = new Adder();
        SetVTableHook(Identifier::bifrost_IAdder_add, adder, PLUGIN_PREFIX(bifrost_IAdder_add__modify_3));
        delete adder;
        break;
      }

      case Mode::VTable_Single_Replace1: {
        IAdder* adder = new Adder();
        SetVTableHook(Identifier::bifrost_IAdder_add, adder, PLUGIN_PREFIX(bifrost_IAdder_add__modify_3));
        SetVTableHook(Identifier::bifrost_IAdder_add, adder, PLUGIN_PREFIX(bifrost_IAdder_add__original_1));
        delete adder;
        break;
      }

      case Mode::VTable_Single_Restore1: {
        IAdder* adder = new Adder();
        SetVTableHook(Identifier::bifrost_IAdder_add, adder, PLUGIN_PREFIX(bifrost_IAdder_add__modify_1));
        RemoveHook(Identifier::bifrost_IAdder_add);
        delete adder;
        break;
      }

      //
      // Batch
      //
      case Mode::Batch_Modify1: {
        IAdder* adder = new Adder();
        std::vector<HookDesc> descs;
        descs.emplace_back(HookDesc{Identifier::bifrost_add, PLUGIN_PREFIX(bifrost_add__modify_1)});
        descs.emplace_back(HookDesc{Identifier::bifrost_IAdder_add, adder, PLUGIN_PREFIX(bifrost_IAdder_add__modify_1)});
        SetHooks(descs.data(), (std::uint32_t)descs.size());
        delete adder;
        break;
      }

      default:
        break;
    }
    WriteToFile(GetFile(), "SetUp" STR(PLUGIN_INDEX), this);
  }

  virtual void TearDown() override { WriteToFile(GetFile(), "TearDown" STR(PLUGIN_INDEX), this); }
};

BIFROST_REGISTER_PLUGIN(PLUGIN_NAME)
