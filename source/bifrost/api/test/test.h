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

#include "bifrost/core/test/test_shared.h"

namespace bifrost {

class TestEnviroment final : public ::testing::Environment, public BaseTestEnviroment {
 public:
  TestEnviroment() { m_logger = std::make_unique<TestLogger>(); }

  /// Get the full path to the injector test executable
  std::wstring GetInjectorExecutable() const;

  /// Get the full path to the injector test plugin
  std::wstring GetInjectorPlugin() const;

  /// Create a temporary file
  std::string GetTmpFile(Context* ctx) const;

  /// Get the test logger
  ILogger* GetLogger() { return m_logger.get(); }

  static TestEnviroment& Get() { return *s_instance; }

  virtual void SetUp() override { s_instance = std::make_unique<TestEnviroment>(); }
  virtual void TearDown() override { s_instance.reset(); }

 private:
  static std::unique_ptr<TestEnviroment> s_instance;
  std::unique_ptr<ILogger> m_logger = nullptr;
};

/// Logging callback
extern void LogCallback(u32 level, const char* module, const char* msg);

}  // namespace bifrost