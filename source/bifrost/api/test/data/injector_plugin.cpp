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

#define BIFROST_NAMESPACE bifrost

#define BIFROST_IMPLEMENTATION
#include "bifrost/template/plugin_main.h"

#include <fstream>
#include <string_view>

class InjectorTestPlugin final : public BIFROST_PLUGIN {
 public:
  // Write the message to the file provided by the arguments to the plugin
  void WriteToFile(std::string_view msg) {
    std::string file = GetArguments();
    std::string m{msg.data(), msg.size()};
    m += ":";

    std::FILE* fp = std::fopen(file.c_str(), "a");
    if (!fp) FatalError(("Failed to open file: \"" + file + "\"").c_str());
    if (std::fwrite(m.c_str(), sizeof(char), m.size(), fp) != m.size()) FatalError(("Failed to write \"" + m + "\" to \"" + file + "\"").c_str());
    if (std::fclose(fp) != 0) FatalError(("Failed to flush and close file: \"" + file + "\"").c_str());
  }

  virtual void SetUp() override { WriteToFile("SetUp"); }

  virtual void TearDown() override { WriteToFile("TearDown"); }

  virtual bool HandleMessage(const void* data, int sizeInBytes) override {
    WriteToFile({(const char*)data, (std::size_t)sizeInBytes});
    return true;
  }

  static const char* Help() { return "Help"; }
};

BIFROST_REGISTER_PLUGIN(InjectorTestPlugin)
BIFROST_REGISTER_PLUGIN_HELP(InjectorTestPlugin::Help)