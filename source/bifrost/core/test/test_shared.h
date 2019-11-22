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

#define SPDLOG_NO_THREAD_ID
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/msvc_sink.h>

namespace bifrost {

class TestLogger final : public ILogger {
 public:
  TestLogger() {
    auto consoleSink = std::make_shared<spdlog::sinks::stderr_color_sink_st>();
    consoleSink->set_color(spdlog::level::trace, 8);
    consoleSink->set_color(spdlog::level::debug, 7);
    consoleSink->set_color(spdlog::level::info, 15);
    consoleSink->set_color(spdlog::level::warn, 14);
    consoleSink->set_color(spdlog::level::err, 12);

    auto msvcSink = std::make_shared<spdlog::sinks::msvc_sink_st>();
    std::vector<spdlog::sink_ptr> sinkVec = {consoleSink, msvcSink};

    try {
      m_logger = std::make_shared<spdlog::logger>("bifrost_test", sinkVec.begin(), sinkVec.end());
      m_logger->set_level(spdlog::level::trace);
      m_logger->set_pattern("[%H:%M:%S.%e] [%-5l] %v");
    } catch (const spdlog::spdlog_ex& ex) {
      throw std::runtime_error(fmt::format("Cannot create logger: {}", ex.what()));
    }
  }

  virtual void SetModule(const char* module) override {
    BIFROST_LOCK_GUARD(m_mutex);
    m_module = module;
  }

  virtual void Sink(LogLevel level, const char* module, const char* msg) override {
    if (!m_logger) return;

    auto logLevel = (ILogger::LogLevel)level;
    auto spdLevel = spdlog::level::off;

    switch (logLevel) {
      case ILogger::LogLevel::Trace:
        spdLevel = spdlog::level::trace;
        break;
      case ILogger::LogLevel::Debug:
        spdLevel = spdlog::level::debug;
        break;
      case ILogger::LogLevel::Info:
        spdLevel = spdlog::level::info;
        break;
      case ILogger::LogLevel::Warn:
        spdLevel = spdlog::level::warn;
        break;
      case ILogger::LogLevel::Error:
        spdLevel = spdlog::level::err;
        break;
      case ILogger::LogLevel::Disable:
      default:
        spdLevel = spdlog::level::off;
        break;
    }

    BIFROST_LOCK_GUARD(m_mutex);
    m_buffer.clear();
    if (!std::string_view(module).empty()) {
      m_buffer += "[";
      m_buffer += module;
      m_buffer += "] ";
    }
    m_buffer += msg;
    m_logger->log(spdLevel, m_buffer.c_str());
  }
  virtual void Sink(LogLevel level, const char* msg) override { Sink(level, m_module.empty() ? "" : m_module.c_str(), msg); }

 private:
  std::mutex m_mutex;
  std::string m_module;

  std::shared_ptr<spdlog::logger> m_logger;
  std::string m_buffer;
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

    std::vector<std::filesystem::path> paths{curPath / filename};
    std::vector<std::filesystem::path> subPaths;

#ifdef BIFROST_CONFIG_DEBUG
    subPaths.emplace_back(L"bin\\Debug");
    paths.emplace_back(curPath / L"bin\\Debug" / filename);
#else
    subPaths.emplace_back(L"bin\\Release");
    paths.emplace_back(curPath / L"bin\\Release" / filename);
#endif

    for (auto& p : std::filesystem::directory_iterator(curPath)) {
      if (p.is_directory()) {
        for (auto& subPath : subPaths) {
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
