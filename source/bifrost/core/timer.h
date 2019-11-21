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

namespace bifrost {

/// High resolution timer
class Timer {
 public:
  /// Start the timer
  Timer() : m_start(std::chrono::high_resolution_clock::now()) {}

  /// Reset the timer to the current time
  inline void Start() noexcept { m_start = std::chrono::high_resolution_clock::now(); }

  /// Return the number of milliseconds elapsed since the timer was last reset via `start()`
  inline std::size_t Stop() noexcept {
    auto end = std::chrono::high_resolution_clock::now();
    return static_cast<std::size_t>(std::chrono::duration<double, std::milli>(end - m_start).count());
  }

 private:
  std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
};

}  // namespace bifrost