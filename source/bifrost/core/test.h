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
#include <gtest/gtest.h>

namespace bifrost {

class TestLogger final : public ILogger {
 public:
  virtual void SetModule(const char* module) override;

 protected:
  virtual void Sink(LogLevel level, const char* msg) override;

 private:
  std::mutex m_mutex;
  std::string m_module;
};

template <bool UseSharedMemory>
class TestBase : public ::testing::Test {
 public:
  TestBase() { m_logger = std::make_unique<TestLogger>(); }

  void SetUp() override {
    m_context = std::make_unique<Context>();
    m_context->SetLogger(m_logger.get());

    if (UseSharedMemory) {
      m_memory = std::make_unique<SharedMemory>(m_context.get(), "bifrost::test::memory", 1 << 20);
      m_context->SetSharedMemory(m_memory.get());
    }
  }
  void TearDown() override {
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

  Context* GetContext() { return m_context.get(); }

 private:
  std::unique_ptr<ILogger> m_logger = nullptr;
  std::unique_ptr<ISharedMemory> m_memory = nullptr;
  std::unique_ptr<Context> m_context = nullptr;
};

}  // namespace bifrost
