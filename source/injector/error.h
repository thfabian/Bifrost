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

#include "injector/common.h"

namespace injector {

/// Storage of last error message
class Error {
 public:
  /// Get singleton instance
  static Error& Get();

  /// Set the name of the program
  void SetProgram(const std::string& program);

  /// Issue a critical error and exit
  template <class... Args>
  [[noreturn]] void Critical(const char* fmt, Args&&... args);

  /// Issue a warning
  template <class... Args>
  void Warning(const char* fmt, Args&&... args);

 private:
  static std::unique_ptr<Error> m_instance;
  std::string m_program;
};

template <class... Args>
void Error::Critical(const char* fmt, Args&&... args) {
  assert(!m_program.empty());
  std::fprintf(stderr, bifrost::StringFormat("%s: error: %s", m_program.c_str(), bifrost::StringFormat(fmt, args...).c_str()).c_str());
  std::fflush(stderr);
  std::exit(1);
}

template <class... Args>
void Error::Warning(const char* fmt, Args&&... args) {
  assert(!m_program.empty());
  std::fprintf(stderr, bifrost::StringFormat("%s: warning: %s", m_program.c_str(), bifrost::StringFormat(fmt, args...).c_str()).c_str());
}

}  // namespace injector