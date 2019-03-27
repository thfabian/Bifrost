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

#include "bifrost_core/common.h"

namespace bifrost {

#pragma pack(push)
#pragma pack(1)

/// 4 byte read/write spin lock
class SpinMutex {
 public:
  SpinMutex() : m_lock(0){};

  SpinMutex(const SpinMutex&) = delete;
  SpinMutex& operator=(const SpinMutex&) = delete;

  SpinMutex(SpinMutex&&) = default;
  SpinMutex& operator=(SpinMutex&&) = default;

  inline void lock() noexcept {
    while (InterlockedExchange(&m_lock, 1) == 1) {
    }
  }

  inline void unlock() noexcept { InterlockedExchange(&m_lock, 0); }

  inline bool try_lock() noexcept { return (InterlockedExchange(&m_lock, 1) == 0); }

 private:
  volatile u32 m_lock;
};

#pragma pack(pop)

}  // namespace bifrost

/// RAII construct to lock/unlock the `mutex`
#define BIFROST_LOCK_GUARD(mutex) std::lock_guard<std::decay<decltype(mutex)>::type> bifrost_lock_guard_##__LINE__(mutex)