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
#include "bifrost/core/ptr.h"

namespace bifrost {

/// Shared memory hash map
template <class KeyT, class ValueT>
class SMHashMap final : public Object {
 public:
  static constexpr u64 InitialSize = 256;
  static constexpr u64 MaxChainLength = 8;

  struct Node {
    KeyT Key;
    bool InUse;
    ValueT Data;
  };

  /// Create an empty hash map
  SMHashMap();
  ~SMHashMap();

 private:
  u32 m_tableSize;
  u32 m_size;
  Ptr<Node> m_data;
};

template <class KeyT, class ValueT>
SMHashMap<KeyT, ValueT>::SMHashMap() {
  m_data = New<Node>(this, InitialSize);
}

template <class KeyT, class ValueT>
SMHashMap<KeyT, ValueT>::~SMHashMap() {}

}  // namespace bifrost