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
#include "bifrost/core/module_loader.h"
#include "bifrost/core/shared_memory.h"

namespace bifrost {

class TestEnviroment final : public ::testing::Environment, public BaseTestEnviroment {
 public:
  /// Get the full path to the mock executable
  std::wstring GetMockExecutable() const;

  /// Get the full path to the mock dll
  std::wstring GetMockDll() const;

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
  std::unique_ptr<SharedMemory> CreateSharedMemory(u64 size = 1 << 14, const char* name = nullptr) {
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

using TestBaseNoSharedMemory = TestBase<false>;
using TestBaseSharedMemory = TestBase<true>;

}  // namespace bifrost
