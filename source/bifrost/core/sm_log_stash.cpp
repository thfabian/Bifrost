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
#include "bifrost/core/sm_log_stash.h"
#include "bifrost/core/ilogger.h"

namespace bifrost {

void SMLogStash::Destruct(SharedMemory* mem) { m_messageQueue.Destruct(mem); }

bool SMLogStash::Empty() {
  BIFROST_LOCK_GUARD(m_mutex);
  return m_messageQueue.Empty();
}

void SMLogStash::Push(Context* ctx, u32 level, const char* module, const char* message) {
  SMLogMessage msg{level, {ctx, module == nullptr ? "" : module}, {ctx, message == nullptr ? "" : message}};
  BIFROST_LOCK_GUARD(m_mutex);
  m_messageQueue.PushBack(ctx, std::move(msg));
}

bool SMLogStash::TryPop(Context* ctx, LogMessage& msg) {
  SMLogMessage* newMsg = nullptr;

  // Get the front node
  {
    BIFROST_LOCK_GUARD(m_mutex);
    newMsg = m_messageQueue.PeekFront(ctx);
    if (!newMsg) return false;
    m_messageQueue.PopFront(ctx, true);
  }

  // Copy the message out of shared memory
  msg.Level = newMsg->Level;
  msg.Module = newMsg->Module.AsView(ctx);
  msg.Message = newMsg->Message.AsView(ctx);

  // Delete the message (we delayed the delete during PopFront)
  Delete(ctx, Ptr<SMLogMessage>::FromAddress(newMsg, ctx->Memory().GetBaseAddress()));
  return true;
}

u64 SMLogStash::Size(Context* ctx) {
  BIFROST_LOCK_GUARD(m_mutex);
  return m_messageQueue.Size(ctx);
}

LogStashConsumer::LogStashConsumer(Context* ctx, SMLogStash* logStash, ILogger* sink) {
  m_consumerThread = std::thread([this, ctx, logStash, sink]() {
    std::array<u64, 5> timeoutInMs{1, 5, 10, 50, 100};
    u64 curTimeoutInMsIndex = 0;

    SMLogStash::LogMessage m_buffer;
    m_buffer.Message.reserve(1024);
    m_buffer.Module.reserve(1024);

    while (!m_done || !logStash->Empty()) {
      if (logStash->Empty()) {
        // No messages.. sleep
        ::Sleep((DWORD)timeoutInMs[curTimeoutInMsIndex]);
        curTimeoutInMsIndex = std::min(curTimeoutInMsIndex + 1, timeoutInMs.size() - 1);
      } else {
        // Try to pop one message
        if (logStash->TryPop(ctx, m_buffer)) {
          sink->Sink((ILogger::LogLevel)m_buffer.Level, m_buffer.Module.c_str(), m_buffer.Message.c_str());
          curTimeoutInMsIndex = 0;
        }
      }
    }
  });
}

LogStashConsumer::~LogStashConsumer() { StopAndFlush(); }

void LogStashConsumer::StopAndFlush() {
  if (!m_done) {
    m_done = true;
    m_consumerThread.join();
  }
}

}  // namespace bifrost