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
#include "bifrost/core/process.h"

namespace {

using namespace bifrost;

class ProcessTest : public TestBaseNoSharedMemory {};

TEST_F(ProcessTest, Wait) {
  auto exe = TestEnviroment::Get().GetMockExecutable();

  {
    Process proc(GetContext(), {exe, {"0"}, false});
    ASSERT_NO_THROW(proc.Wait());

    ASSERT_NE(nullptr, proc.ExitCode());
    EXPECT_EQ(0, *proc.ExitCode());
  }

  {
    Process proc(GetContext(), {exe, {"0", "500"}, false});
    ASSERT_NO_THROW(proc.Wait());

    ASSERT_NE(nullptr, proc.ExitCode());
    EXPECT_EQ(0, *proc.ExitCode());
  }
}

TEST_F(ProcessTest, ExitCode) {
  auto exe = TestEnviroment::Get().GetMockExecutable();

  {
    Process proc(GetContext(), {exe, {"0"}, false});
    ASSERT_NO_THROW(proc.Wait());

    ASSERT_NE(nullptr, proc.ExitCode());
    EXPECT_EQ(0, *proc.ExitCode());
  }

  {
    Process proc(GetContext(), {exe, {"5"}, false});
    ASSERT_NO_THROW(proc.Wait());

    ASSERT_NE(nullptr, proc.ExitCode());
    EXPECT_EQ(5, *proc.ExitCode());
  }
}

TEST_F(ProcessTest, Poll) {
  auto exe = TestEnviroment::Get().GetMockExecutable();

  Process proc(GetContext(), {exe, {"0", "500"}, false});
  ASSERT_FALSE(proc.Poll());  // This could in theory fail but it's incredibly unlikely
  while (!proc.Poll()) {
    ::Sleep(50);
  }

  // Process returned - should return
  ASSERT_TRUE(proc.Poll());
  ASSERT_TRUE(proc.Poll());

  ASSERT_NE(nullptr, proc.ExitCode());
  EXPECT_EQ(0, *proc.ExitCode());
}

TEST_F(ProcessTest, Resume) {
  auto exe = TestEnviroment::Get().GetMockExecutable();

  Process proc(GetContext(), {exe, {"1", "500"}, true});
  ASSERT_FALSE(proc.Poll());

  ASSERT_NO_THROW(proc.Resume());
  ASSERT_NO_THROW(proc.Wait());

  ASSERT_NE(nullptr, proc.ExitCode());
  EXPECT_EQ(1, *proc.ExitCode());
}

}  // namespace
