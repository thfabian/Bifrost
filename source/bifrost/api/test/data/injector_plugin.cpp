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

#define BIFROST_IMPLEMENTATION
#include "bifrost/template/plugin_main.h"

class InjectorTestPlugin final : public ::bifrost::Plugin {
 public:
  virtual void SetUp() override { Log(Plugin::LogLevel::Error, "Damn son!"); }
  virtual void TearDown() override { Log(Plugin::LogLevel::Error, "Doneso!"); }
};

BIFROST_REGISTER_PLUGIN(InjectorTestPlugin)