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

//
// This file contains shared functions between the executable, dll and test plugins.
//

#pragma once

#include <cstdlib>
#include <exception>
#include <stdexcept>
#include <string>
#include <string_view>

namespace bifrost {

namespace internal {

template <class ErrorFuncT>
inline void WriteToFileImpl(ErrorFuncT&& error, std::string file, std::string_view msg) {
  std::string m{msg.data(), msg.size()};
  m += ":";

  std::FILE* fp = std::fopen(file.c_str(), "a");
  if (!fp) error("Failed to open file: \"" + file + "\"");
  if (std::fwrite(m.c_str(), sizeof(char), m.size(), fp) != m.size()) error("Failed to write \"" + m + "\" to \"" + file + "\"");
  if (std::fclose(fp) != 0) error("Failed to flush and close file: \"" + file + "\"");
}

}  // namespace internal

/// Write `msg` to `file`
template <class PluginT>
inline void WriteToFile(std::string file, std::string_view msg, PluginT* plugin) {
  internal::WriteToFileImpl([&plugin](std::string str) { return plugin->FatalError(str.c_str()); }, file, msg);
}

inline void WriteToFile(std::string file, std::string_view msg) {
  internal::WriteToFileImpl([](std::string str) { throw std::runtime_error(str.c_str()); }, file, msg);
}

}  // namespace bifrost
