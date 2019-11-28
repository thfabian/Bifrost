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
#include "bifrost/core/object.h"
#include "bifrost/core/type.h"
#include "bifrost/core/thread.h"
#include "bifrost/core/non_copyable.h"

namespace bifrost {

class Process;
class Context;

/// Thread abstraction
class Thread : public Object, public NonCopyable {
 public:
  /// Open the thread given by the thread id `id`
  Thread(Context* ctx, u32 id);

  Thread(const Thread&) = delete;
  Thread& operator=(const Thread&) = delete;

  Thread(Thread&&) = default;
  Thread& operator=(Thread&&) = default;
  ~Thread();

  /// Suspend the thread
  void Suspend(bool verbose = true);

  /// Resume the thread
  void Resume(bool verbose = true);

  /// Get the thread handle
  HANDLE Handle() const noexcept;

  /// Get the thread identifier
  u32 Id() const noexcept;

  /// Resume all threads
  static void ResumeAll(std::vector<std::unique_ptr<Thread>>& threads, bool verbose = true);

  /// Suspend all threads
  static void SuspendAll(std::vector<std::unique_ptr<Thread>>& threads, bool verbose = true);

 private:
  u32 m_id;
  HANDLE m_hThread = INVALID_HANDLE_VALUE;
};

/// RAII construct for suspending all threads (besides the calling thread) of this process
class ThreadSuspender {
 public:
  ThreadSuspender(std::vector<std::unique_ptr<Thread>>* threads);
  ~ThreadSuspender();

 private:
  std::vector<std::unique_ptr<Thread>>* m_threads;
};

inline bool operator==(const Thread& a, const Thread& b) { return a.Id() == b.Id(); }
inline bool operator!=(const Thread& a, const Thread& b) { return !(a == b); }

}  // namespace bifrost
