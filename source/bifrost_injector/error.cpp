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

#include "bifrost_injector/common.h"
#include "bifrost_injector/error.h"

namespace bifrost::injector {

std::unique_ptr<Error> Error::m_instance = nullptr;

Error& Error::Get() {
  if (!m_instance) {
    m_instance = std::make_unique<Error>();
  }
  return *m_instance;
}

void Error::SetProgram(const std::string& program) { m_program = program; }

}  // namespace bifrost::injector
