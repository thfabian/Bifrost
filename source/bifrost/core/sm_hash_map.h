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
#include "bifrost/core/ptr.h"
#include "bifrost/core/sm_new.h"

namespace bifrost {

/// Shared memory hash map
template <class KeyT, class ValueT>
class SMHashMap : public SMObject {
 public:
  static constexpr u32 MaxChainLength = 8;
  static constexpr i32 Invalid = -1;

  struct Node {
    KeyT Key;
    ValueT Value;
  };
  struct InternalNode {
    Node Node;
    bool InUse = false;
  };

  /// Create an empty hash map
  SMHashMap(Context* ctx, u32 initialCapacity = 16) {
    m_capacity = initialCapacity;
    m_size = 0;
    m_data = NewArray<InternalNode>(ctx, m_capacity);
  }

  /// Destruct the map
  void Destruct(SharedMemory* mem) { DeleteArray(mem, m_data, m_capacity); }

  /// Get the value of element with key `k` or NULL if no such key exists
  const ValueT* Get(Context* ctx, const KeyT& k) const {
    i32 idx = HashKey(k);

    // Linear probing
    InternalNode* data = Resolve(ctx, m_data);
    for (u32 i = 0; i < MaxChainLength; i++) {
      if (data[idx].InUse && data[idx].Node.Key == k) return &data[idx].Node.Value;
      idx = (idx + 1) % m_capacity;
    }
    return nullptr;
  }

  /// Insert the element with key ``k`` and value ``v``
  Node* Insert(Context* ctx, const KeyT& k, ValueT v) {
    i32 index = Hash(ctx, k);
    while (index == Invalid) {
      Rehash(ctx);
      index = Hash(ctx, k);
    }

    // Set the data
    InternalNode* data = Resolve(ctx, m_data);
    data[index].Node.Key = k;
    data[index].Node.Value = std::move(v);
    data[index].InUse = true;
    m_size += 1;

    return &data[index].Node;
  }

  /// Remove the key `k`
  void Remove(Context* ctx, const KeyT& k) {
    i32 idx = HashKey(k);

    // Linear probing
    InternalNode* data = Resolve(ctx, m_data);
    for (u32 i = 0; i < MaxChainLength; i++) {
      if (data[idx].InUse && data[idx].Node.Key == k) {
        data[idx].InUse = false;
        --m_size;
        return;
      }
      idx = (idx + 1) % m_capacity;
    }
  }

  /// Get the size of the map
  u32 Size() const { return m_size; }

  /// Get the capacity of the map
  u32 Capacity() const { return m_capacity; }

 private:
  i32 Hash(Context* ctx, const KeyT& key) const {
    if (m_size >= (m_capacity / 2)) return Invalid;

    InternalNode* data = Resolve(ctx, m_data);

    // Find the best index
    i32 idx = HashKey(key);

    // Linear probing
    for (u32 i = 0; i < MaxChainLength; i++) {
      if (!data[idx].InUse) return idx;
      if (data[idx].InUse && data[idx].Node.Key == key) return idx;
      idx = (idx + 1) % m_capacity;
    }

    return Invalid;
  }

  i32 HashKey(const KeyT& key) const {
    auto hasher = std::hash<KeyT>();
    std::size_t hash = hasher(key);

    // Robert Jenkins' 32 bit Mix Function
    hash += (hash << 12);
    hash ^= (hash >> 22);
    hash += (hash << 4);
    hash ^= (hash >> 9);
    hash += (hash << 10);
    hash ^= (hash >> 2);
    hash += (hash << 7);
    hash ^= (hash >> 12);

    // Knuth's Multiplicative Method
    hash = (hash >> 3) * 2654435761;
    return hash % m_capacity;
  }

  void Rehash(Context* ctx) {
    // Update the array
    auto oldData = m_data;
    m_data = NewArray<InternalNode>(ctx, 2 * m_capacity);

    // Update the size
    u32 oldTableSize = m_capacity;
    m_capacity = 2 * m_capacity;
    m_size = 0;

    // Rehash the elements
    InternalNode* oldDataP = Resolve(ctx, oldData);
    for (u32 i = 0; i < oldTableSize; i++) {
      if (!oldDataP[i].InUse) continue;
      Insert(ctx, oldDataP[i].Node.Key, std::move(oldDataP[i].Node.Value));
    }
    DeleteArray(ctx, oldData, oldTableSize);
  }

 private:
  Ptr<InternalNode> m_data;
  u32 m_capacity;
  u32 m_size;
};

}  // namespace bifrost