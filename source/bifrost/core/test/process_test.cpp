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

class ProcessTest : public TestBaseNoSharedMemory {
 public:
  virtual void SetUp() override {
    TestBase<false>::SetUp();
    KillProcess(GetContext(), std::filesystem::path(TestEnviroment::Get().GetMockExecutable()).filename().native());
  }
};

TEST_F(ProcessTest, Wait) {
  auto exe = TestEnviroment::Get().GetMockExecutable();

  {
    Process proc(GetContext(), {exe, "0", false});
    ASSERT_NO_THROW(proc.Wait());

    ASSERT_NE(nullptr, proc.GetExitCode());
    EXPECT_EQ(0, *proc.GetExitCode());
  }

  {
    Process proc(GetContext(), {exe, "0 200", false});
    ASSERT_NO_THROW(proc.Wait());

    ASSERT_NE(nullptr, proc.GetExitCode());
    EXPECT_EQ(0, *proc.GetExitCode());
  }
}

TEST_F(ProcessTest, ExitCode) {
  auto exe = TestEnviroment::Get().GetMockExecutable();

  {
    Process proc(GetContext(), {exe, "0", false});
    ASSERT_NO_THROW(proc.Wait());

    ASSERT_NE(nullptr, proc.GetExitCode());
    EXPECT_EQ(0, *proc.GetExitCode());
  }

  {
    Process proc(GetContext(), {exe, "5", false});
    ASSERT_NO_THROW(proc.Wait());

    ASSERT_NE(nullptr, proc.GetExitCode());
    EXPECT_EQ(5, *proc.GetExitCode());
  }
}

TEST_F(ProcessTest, Poll) {
  auto exe = TestEnviroment::Get().GetMockExecutable();

  Process proc(GetContext(), {exe, "0 200", false});
  ASSERT_FALSE(proc.Poll());  // This could in theory fail but it's incredibly unlikely
  while (!proc.Poll()) {
    ::Sleep(50);
  }

  // Process returned - should return
  ASSERT_TRUE(proc.Poll());
  ASSERT_TRUE(proc.Poll());

  ASSERT_NE(nullptr, proc.GetExitCode());
  EXPECT_EQ(0, *proc.GetExitCode());
}
TEST_F(ProcessTest, ResumeAndSuspend) {
  auto exe = TestEnviroment::Get().GetMockExecutable();

  Process proc(GetContext(), {exe, "1 200", true});
  ASSERT_FALSE(proc.Poll());

  auto threads = proc.GetThreads();
	ASSERT_EQ(threads.size(), 1); // should only have 1 thread (main thread)
  
	ASSERT_NO_THROW(proc.Suspend(threads));  // thread should already be suspended -> count = 2
  ::Sleep(250);
  ASSERT_FALSE(proc.Poll());

  ASSERT_NO_THROW(proc.Resume(threads));  // thread should still be suspended -> count = 1
  ::Sleep(250);
  ASSERT_FALSE(proc.Poll());

  ASSERT_NO_THROW(proc.Resume(threads));  // thread should now run -> cound = 0
  ASSERT_NO_THROW(proc.Suspend(threads));  // suspend running thread
  ::Sleep(250);
  ASSERT_FALSE(proc.Poll());

  ASSERT_NO_THROW(proc.Resume(threads));
  ASSERT_NO_THROW(proc.Resume(threads));  // thread is already running, should be nop
  ASSERT_NO_THROW(proc.Wait());

  ASSERT_NE(nullptr, proc.GetExitCode());
  EXPECT_EQ(1, *proc.GetExitCode());
}

TEST_F(ProcessTest, ConnectPid) {
  auto exe = TestEnviroment::Get().GetMockExecutable();
  Process mproc(GetContext(), {exe, "1 100", true});
  Process cproc(GetContext(), mproc.GetPid());

  ASSERT_EQ(mproc.GetPid(), cproc.GetPid());
  ASSERT_EQ(mproc.GetThreads(), cproc.GetThreads());

  EXPECT_FALSE(mproc.Poll());
  EXPECT_FALSE(cproc.Poll());
  EXPECT_EQ(nullptr, mproc.GetExitCode());
  EXPECT_EQ(nullptr, cproc.GetExitCode());

  ASSERT_NO_THROW(cproc.Resume(cproc.GetThreads()));  // Let the connected thread resume the process
  ASSERT_NO_THROW(cproc.Wait());

  EXPECT_NE(nullptr, mproc.GetExitCode());
  EXPECT_NE(nullptr, cproc.GetExitCode());
  EXPECT_EQ(1, *mproc.GetExitCode());
  EXPECT_EQ(1, *cproc.GetExitCode());
}

TEST_F(ProcessTest, ConnectName) {
  auto exe = TestEnviroment::Get().GetMockExecutable();
  Process mproc(GetContext(), {exe, "1 100", true});
  Process cproc(GetContext(), std::filesystem::path(TestEnviroment::Get().GetMockExecutable()).filename().native());

  ASSERT_EQ(mproc.GetPid(), cproc.GetPid());
  ASSERT_EQ(mproc.GetThreads(), cproc.GetThreads());

  EXPECT_FALSE(mproc.Poll());
  EXPECT_FALSE(cproc.Poll());
  EXPECT_EQ(nullptr, mproc.GetExitCode());
  EXPECT_EQ(nullptr, cproc.GetExitCode());

  ASSERT_NO_THROW(cproc.Resume(cproc.GetThreads()));  // Let the connected thread resume the process
  ASSERT_NO_THROW(cproc.Wait());

  EXPECT_NE(nullptr, mproc.GetExitCode());
  EXPECT_NE(nullptr, cproc.GetExitCode());
  EXPECT_EQ(1, *mproc.GetExitCode());
  EXPECT_EQ(1, *cproc.GetExitCode());
}

TEST_F(ProcessTest, InjectSuccess) {
  auto exe = TestEnviroment::Get().GetMockExecutable();
  auto dll = TestEnviroment::Get().GetMockDll();

  Process proc(GetContext(), {exe, "0 200", true});

  ASSERT_NO_THROW(proc.Inject({dll}));

  ASSERT_NO_THROW(proc.Resume(proc.GetThreads()));
  ASSERT_NO_THROW(proc.Wait());

  ASSERT_NE(nullptr, proc.GetExitCode());
  EXPECT_EQ(0, *proc.GetExitCode());
}

TEST_F(ProcessTest, InjectFail) {
  auto exe = TestEnviroment::Get().GetMockExecutable();
  auto dll = TestEnviroment::Get().GetMockDll();

  Process proc(GetContext(), {exe, "0 200", true});

  ASSERT_THROW(proc.Inject({L"invalid"}), std::runtime_error);

  ASSERT_NO_THROW(proc.Resume(proc.GetThreads()));
  ASSERT_NO_THROW(proc.Wait());

  ASSERT_NE(nullptr, proc.GetExitCode());
  EXPECT_EQ(0, *proc.GetExitCode());
}

TEST_F(ProcessTest, InjectUnsuspended) {
  auto exe = TestEnviroment::Get().GetMockExecutable();
  auto dll = TestEnviroment::Get().GetMockDll();

  Process proc(GetContext(), {exe, "0 200", false});

  ASSERT_NO_THROW(proc.Inject({dll}));
  ASSERT_NO_THROW(proc.Wait());

  ASSERT_NE(nullptr, proc.GetExitCode());
  EXPECT_EQ(0, *proc.GetExitCode());
}

TEST_F(ProcessTest, InjectInit) {
  auto exe = TestEnviroment::Get().GetMockExecutable();
  auto dll = TestEnviroment::Get().GetMockDll();

  auto ctx = GetContext();
  auto mem = CreateSharedMemory();
  ctx->SetMemory(mem.get());

  Process proc(ctx, {exe, "0 200", true});

  ASSERT_NO_THROW(proc.Inject({dll, "MockDllInit", "foo"}));

  ASSERT_NO_THROW(proc.Resume(proc.GetThreads()));
  ASSERT_NO_THROW(proc.Wait());

  ASSERT_NE(nullptr, proc.GetExitCode());
  EXPECT_EQ(0, *proc.GetExitCode());
}

}  // namespace
