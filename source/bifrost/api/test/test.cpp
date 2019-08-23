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

#include "bifrost/api/test/test.h"

namespace bifrost {

std::unique_ptr<TestEnviroment> TestEnviroment::s_instance = nullptr;

std::wstring TestEnviroment::GetInjectorExecutable() const { return GetFile(L"executable", L"test-bifrost-api-injector-executable.exe"); }

std::wstring TestEnviroment::GetInjectorPlugin() const { return GetFile(L"plugin", L"test-bifrost-api-injector-plugin.dll"); }

void LogCallback(u32 level, const char* module, const char* msg) { TestEnviroment::Get().GetLogger()->Sink((ILogger::LogLevel)level, module, msg); }

}  // namespace bifrost
