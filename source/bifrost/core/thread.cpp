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

#include "bifrost/core/common.h"

#include "bifrost/core/thread.h"
#include "bifrost/core/process.h"
#include "bifrost/core/error.h"
#include "bifrost/core/util.h"
#include "bifrost/core/exception.h"

namespace bifrost {

Thread::Thread(Context* ctx, u32 id) : Object(ctx) {
  m_id = id;
  BIFROST_ASSERT_WIN_CALL_MSG_CTX(ctx, (m_hThread = ::OpenThread(THREAD_ALL_ACCESS, FALSE, m_id)) != NULL,
                                  StringFormat("Failed to open thread %u", m_id).c_str());
}

Thread::~Thread() {
  if (m_hThread) BIFROST_CHECK_WIN_CALL(::CloseHandle(m_hThread) != FALSE);
}

void Thread::Suspend(bool verbose /*= true*/) {
  if (verbose) Logger().DebugFormat("Suspending thread %u ...", m_id);

  DWORD suspendCount = 0;
  BIFROST_ASSERT_WIN_CALL((suspendCount = ::SuspendThread(m_hThread)) != ((DWORD)-1));

  if (verbose) {
    Logger().DebugFormat("Successfully suspended thread %u, thread %s (previous suspend count %u)", m_id,
                         suspendCount == 0 ? "was running" : "was already suspended", suspendCount);
  }
}

void Thread::Resume(bool verbose /*= true*/) {
  if (verbose) Logger().DebugFormat("Resuming thread %u ...", m_id);

  DWORD suspendCount = 0;
  BIFROST_ASSERT_WIN_CALL((suspendCount = ::ResumeThread(m_hThread)) != ((DWORD)-1));

  if (verbose) {
    Logger().DebugFormat("Successfully resumed thread %u, thread is %s (previous suspend count %u)", m_id,
                         suspendCount <= 1 ? "now running" : "still suspended", suspendCount);
  }
}

HANDLE Thread::Handle() const noexcept { return m_hThread; }

u32 Thread::Id() const noexcept { return m_id; }

void Thread::ResumeAll(std::vector<std::unique_ptr<Thread>>& threads, bool verbose) {
  for (auto& thread : threads) thread->Resume(verbose);
}

void Thread::SuspendAll(std::vector<std::unique_ptr<Thread>>& threads, bool verbose) {
  for (auto& thread : threads) thread->Suspend(verbose);
}

ThreadSuspender::ThreadSuspender(std::vector<std::unique_ptr<Thread>>* threads) : m_threads(threads) { Thread::SuspendAll(*m_threads, false); }

ThreadSuspender::~ThreadSuspender() { Thread::ResumeAll(*m_threads, false); }

}  // namespace bifrost