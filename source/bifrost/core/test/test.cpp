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

#include "bifrost/core/test/test.h"

namespace bifrost {

std::unique_ptr<TestEnviroment> TestEnviroment::s_instance = nullptr;

std::wstring TestEnviroment::GetMockExecutable() const { return GetFile(L"executable", L"test-bifrost-core-mock-executable.exe"); }

std::wstring TestEnviroment::GetMockDll() const { return GetFile(L"dll", L"test-bifrost-core-mock-dll.dll"); }

}  // namespace bifrost
