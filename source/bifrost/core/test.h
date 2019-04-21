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
#include "bifrost/core/shared_memory.h"
#include "bifrost/core/module_loader.h"
#include <gtest/gtest.h>

namespace bifrost {

class TestLogger final : public ILogger {
 public:
  virtual void SetModule(const char* module) override;

  virtual void Sink(LogLevel level, const char* module, const char* msg) override;
  virtual void Sink(LogLevel level, const char* msg) override;

 private:
  std::mutex m_mutex;
  std::string m_module;
};

class TestEnviroment final : public ::testing::Environment {
 public:
  /// Name of the current test-case
  std::string TestCaseName() const;

  /// Name of the current test
  std::string TestName() const;

  static TestEnviroment& Get() { return *s_instance; }

  virtual void SetUp() override { s_instance = std::make_unique<TestEnviroment>(); }
  virtual void TearDown() override { s_instance.reset(); }

 private:
  static std::unique_ptr<TestEnviroment> s_instance;
};

template <bool UseSharedMemory>
class TestBase : public ::testing::Test {
 public:
  TestBase() { m_logger = std::make_unique<TestLogger>(); }

  /// Create shared memory (if `name` is NULL the current test case and test name is used
  std::unique_ptr<SharedMemory> CreateSharedMemory(u64 size = 1 << 12, const char* name = nullptr) {
    auto smName = name == nullptr ? TestEnviroment::Get().TestCaseName() + "." + TestEnviroment::Get().TestName() : std::string(name);
    return std::make_unique<SharedMemory>(m_context.get(), std::move(smName), size);
  }

  /// Get the test logger
  ILogger* GetLogger() { return m_logger.get(); }

  /// Get the test context
  Context* GetContext() { return m_context.get(); }

  /// Called before each test
  virtual void SetUp() override {
    m_context = std::make_unique<Context>();
    m_context->SetLogger(m_logger.get());

    if (UseSharedMemory) {
      m_memory = CreateSharedMemory();
      m_context->SetMemory(m_memory.get());
    }
  }

  /// Called after each test
  virtual void TearDown() override {
    if (UseSharedMemory) {
      m_memory.reset();
    }
    m_context.reset();
  }

  template <class T>
  T* Resolve(Ptr<T> ptr) {
    BIFROST_ASSERT(UseSharedMemory);
    return ptr.Resolve((void*)m_memory->GetBaseAddress());
  }

 private:
  std::unique_ptr<ILogger> m_logger = nullptr;
  std::unique_ptr<SharedMemory> m_memory = nullptr;
  std::unique_ptr<Context> m_context = nullptr;
};

}  // namespace bifrost
