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

#include "bifrost_shared/common.h"
#include "bifrost_shared/bifrost_shared.h"
#include "bifrost_shared/stl.h"
#include "bifrost_core/non_copyable.h"

namespace bifrost::shared {

/// Shared log stash
class SharedLogStash : public NonCopyable {
 public:
  struct LogMessage {
    u32 Level;
    stl::string Module;
    stl::string Message;
  };

  SharedLogStash();
  ~SharedLogStash();

  /// Push a log message
  bfs_Status Push(int level, const char* module, const char* message);

  /// Set the async state
  bfs_Status SetAsync(bool async);

  /// Register a logging callback
  bfs_Status SetCallback(const char* name, bfs_LogCallback_t loggingCallback);

  /// Remove a logging callback
  bfs_Status RemoveCallback(const char* name);

 private:
  /// Pop a log message
  void Pop();

 private:
  using list_t = stl::deque<LogMessage>;

  // Allocated blocks
  std::mutex m_messageMutex;
  list_t m_messages;

  // Consumer
  bool m_done;
  std::thread m_consumerThread;
  std::condition_variable m_consumerCondVar;

  bool m_asnyc;
  stl::unordered_map<stl::string, bfs_LogCallback_t> m_callbacks;
};

}  // namespace bifrost::shared