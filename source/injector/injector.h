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
#include "injector/process.h"

namespace injector {

class Injector {
 public:
  Injector(std::unique_ptr<Process>&& process);

 private:
  std::unique_ptr<Process> m_process;
};

}  // namespace injector