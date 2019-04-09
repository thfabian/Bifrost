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

#include "bifrost_shared/common.h"
#include "bifrost_shared/shared_log_stash.h"

namespace bifrost::shared {

SharedLogStash::SharedLogStash() : m_done(false), m_asnyc(true) {
  m_consumerThread = std::thread([this]() { Pop(); });
}

SharedLogStash::~SharedLogStash() {
  m_done = true;
  m_consumerCondVar.notify_all();
  m_consumerThread.join();
}

bfs_Status SharedLogStash::SetCallback(const char* name, bfs_LogCallback_t loggingCallback) {
  m_callbacks[name] = loggingCallback;
  return BFS_OK;
}

bfs_Status SharedLogStash::RemoveCallback(const char* name) {
  m_callbacks.erase(name);
  return BFS_OK;
}

bfs_Status SharedLogStash::Push(int level, const char* module, const char* message) {
  if (m_asnyc) {
    LogMessage msg{(u32)level, module == nullptr ? "" : module, message == nullptr ? "" : message};
    std::unique_lock<std::mutex> lock(m_messageMutex);
    m_messages.emplace_back(std::move(msg));
    m_consumerCondVar.notify_all();
  } else {
    for (const auto& cb : m_callbacks) cb.second(level, module == nullptr ? "" : module, message == nullptr ? "" : message);
  }
  return BFS_OK;
}

void SharedLogStash::Pop() {
  while (!m_done || !m_messages.empty()) {
    std::unique_lock<std::mutex> lock(m_messageMutex);
    if (m_messages.empty()) {
      // No messages.. sleep
      m_consumerCondVar.wait(lock, [this]() { return m_done || !m_messages.empty(); });
    } else {
      // Pop one message
      LogMessage msg;
      msg.Level = m_messages.front().Level;
      msg.Message = std::move(m_messages.front().Message);
      msg.Module = std::move(m_messages.front().Module);
      m_messages.pop_front();
      lock.unlock();

      for (const auto& cb : m_callbacks) cb.second(msg.Level, msg.Module.c_str(), msg.Message.c_str());
    }
  }
}

bfs_Status SharedLogStash::SetAsync(bool async) {
  m_asnyc = async;
  return BFS_OK;
}

}  // namespace bifrost::shared