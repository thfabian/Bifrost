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

#include "bifrost/core/common.h"

#include "bifrost/core/context.h"
#include "bifrost/core/ilogger.h"
#include "bifrost/core/util.h"
#include "bifrost/core/mutex.h"
#include <gtest/gtest.h>

namespace bifrost {

class TestLogger final : public ILogger {
 public:
  virtual void SetModule(const char* module) override {
    BIFROST_LOCK_GUARD(m_mutex);
    m_module = module;
  }

  virtual void Sink(LogLevel level, const char* module, const char* msg) override {
    if (level == ILogger::LogLevel::Disable) return;

    // Get current date-time (up to ms accuracy)
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    auto now_ms = now.time_since_epoch();
    auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(now_ms);
    auto tm_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now_ms - now_sec);

    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    struct tm* localTime = std::localtime(&currentTime);

    auto timeStr = StringFormat("%02i:%02i:%02i.%03i", localTime->tm_hour, localTime->tm_min, localTime->tm_sec, tm_ms.count());

    std::stringstream ss;
    ss << "[" << timeStr << "]";

    switch (level) {
      case ILogger::LogLevel::Debug:
        ss << " [DEBUG]";
        break;
      case ILogger::LogLevel::Info:
        ss << " [INFO ]";
        break;
      case ILogger::LogLevel::Warn:
        ss << " [WARN ]";
        break;
      case ILogger::LogLevel::Error:
        ss << " [ERROR]";
        break;
    }

    auto moduleStr = std::string_view(module);
    if (!moduleStr.empty()) ss << StringFormat("[%-20s]", moduleStr.data());

    ss << ": " << msg << std::endl;

    auto outMsg = ss.str();
    std::cout << outMsg;
    ::OutputDebugStringA(outMsg.c_str());
  }
  virtual void Sink(LogLevel level, const char* msg) override { Sink(level, m_module.empty() ? "" : m_module.c_str(), msg); }

 private:
  std::mutex m_mutex;
  std::string m_module;
};

class BaseTestEnviroment {
 public:
  /// Name of the current test-case
  std::string TestCaseName() const {
    const ::testing::TestInfo* testInfo = ::testing::UnitTest::GetInstance()->current_test_info();
    if (testInfo) return testInfo->test_case_name();
    BIFROST_ASSERT(false && "TestCaseName() called outside test");
    return "";
  }

  /// Name of the current test
  std::string TestName() const {
    const ::testing::TestInfo* testInfo = ::testing::UnitTest::GetInstance()->current_test_info();
    if (testInfo) return testInfo->name();
    BIFROST_ASSERT(false && "TestName() called outside test");
    return "";
  }

 protected:
  std::wstring GetFile(std::wstring type, std::wstring filename) const {
    auto curPath = std::filesystem::current_path();

    std::vector<std::filesystem::path> paths{curPath / filename, curPath / L"bin/Debug" / filename, curPath / L"bin/Release" / filename};
    for (auto& p : std::filesystem::directory_iterator(curPath)) {
      if (p.is_directory()) {
        for (const auto subPath : {L"bin/Debug", L"bin/Release"}) {
          paths.emplace_back(p / std::filesystem::path(subPath) / filename);
        }
      }
    }

    for (const auto& p : paths) {
      if (std::filesystem::exists(p)) return p.native();
    }

    std::wstringstream ss;
    ss << type << " file \"" << filename << "\" not present, invalid paths:\n";
    for (const auto& p : paths) ss << L" - " << p.native() << L"\n";

    throw std::runtime_error(WStringToString(ss.str()));
  };
};

}  // namespace bifrost
