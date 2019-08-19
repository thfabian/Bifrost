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
#include "bifrost/core/mutex.h"
#include "bifrost/core/sm_object.h"
#include "bifrost/core/sm_list.h"
#include "bifrost/core/sm_string.h"

namespace bifrost {

/// Shared log stash - unique per shared memory region (allocated in SMContext)
class SMLogStash : public SMObject {
 public:
  void Destruct(SharedMemory* mem);

  /// System memory log message
  struct LogMessage {
    u32 Level;
    std::string Module;
    std::string Message;
  };

  /// Is the stash empty?
  bool Empty();

  /// Push a new message to the back of the queue
  void Push(Context* ctx, u32 level, const char* module, const char* message);

  /// Try to get the message at the top of the queue and assign it to `msg` - returns true on success
  bool TryPop(Context* ctx, LogMessage& msg);

  /// Size of the stash (only used for testing - O(n))
  u64 Size(Context* ctx);

 private:
  struct SMLogMessage {
    u32 Level;
    SMString Module;
    SMString Message;
  };

  SpinMutex m_mutex;
  SMList<SMLogMessage> m_messageQueue;
};

/// Consume the log stash by forwarding the messages to the underlying logger
class LogStashConsumer {
 public:
  /// Start consuming messages from `logStash` and forward them to `sink`
  LogStashConsumer(Context* ctx, SMLogStash* logStash, ILogger* sink);
  ~LogStashConsumer();

  /// Stop consuming messages - flushes all remaining messages
  void StopAndFlush();

 private:
  bool m_done = false;
  std::thread m_consumerThread;
};

}  // namespace bifrost