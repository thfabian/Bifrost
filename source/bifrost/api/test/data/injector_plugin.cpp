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

#define BIFROST_NAMESPACE injector_plugin
#define BIFROST_PLUGIN_IDENTIFIER
#define BIFROST_PLUGIN_STRING_TO_IDENTIFIER
#define BIFROST_PLUGIN_IDENTIFIER_TO_STRING
#define BIFROST_PLUGIN_IDENTIFIER_TO_FUNCTION_NAME
#define BIFROST_PLUGIN_IDENTIFIER_TO_HOOK_TYPE
#define BIFROST_PLUGIN_IDENTIFIER_TO_VTABLE_OFFSET

#define BIFROST_PLUGIN_OBJECT_TYPE
#define BIFROST_PLUGIN_OBJECT_TYPE_TO_STRING
#define BIFROST_PLUGIN_IDENTIFIER_TO_OBJECT_TYPE

#define BIFROST_PLUGIN_MODULE
#define BIFROST_PLUGIN_IDENTIFIER_TO_MODULE
#define BIFROST_PLUGIN_MODULE_TO_STRING

#define BIFROST_PLUGIN_DSL_DEF
#define BIFROST_PLUGIN_INCLUDES

#define BIFROST_IMPLEMENTATION
#include "bifrost/template/plugin_main.h"

#include "bifrost/api/test/data/shared.h"

using namespace bifrost;

class InjectorTestPlugin final : public ::injector_plugin::Plugin {
 public:
  virtual void SetUp() override { WriteToFile(GetArguments(), "SetUp", this); }
  virtual void TearDown() override { WriteToFile(GetArguments(), "TearDown", this); }

  static const char* Help() { return "Help"; }
};

BIFROST_REGISTER_PLUGIN(InjectorTestPlugin)
BIFROST_REGISTER_PLUGIN_HELP(InjectorTestPlugin::Help)