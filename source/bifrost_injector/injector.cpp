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
#include "bifrost_injector/injector.h"

namespace bifrost::injector {

Injector::Injector(std::unique_ptr<Process>&& process) : m_process(std::move(process)) {}

void Injector::LoadBifrostDll(std::wstring path) {}

}  // namespace bifrost::injector
