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
#include "bifrost/core/sm_type_traits.h"

namespace bifrost {

/// Shared memory hash map
template <class KeyT, class ValueT, class HasherT = SMHasher<KeyT>, class EqualToT = SMEqualTo<KeyT>, class AssignT = SMAssign<KeyT>>
class SMHashMap : public SMObject {
 public:
  static constexpr u32 MaxChainLength = 8;
  static constexpr i32 Invalid = -1;

  struct Node {
    KeyT Key;
    ValueT Value;
  };
  struct SMNode : public SMObject {
    void Destruct(SharedMemory* mem) {
      if(!InUse) return;
      internal::Destruct(mem, &Node.Key);
      internal::Destruct(mem, &Node.Value);
    }

    Node Node;
    bool InUse = false;
  };

  /// Create an empty hash map
  SMHashMap() {
    m_capacity = 0;
    m_size = 0;
  }

  /// Create an empty hash with given capacity
  SMHashMap(Context* ctx, u32 capacity) {
    m_capacity = capacity;
    m_size = 0;
    m_data = NewArray<SMNode>(ctx, m_capacity);
  }

  /// Destruct the map
  void Destruct(SharedMemory* mem) { DeleteArray(mem, m_data, m_capacity); }

  /// Get the value of element with key `k` or NULL if no such key exists
  const ValueT* Get(Context* ctx, const KeyT& k) const {
    i32 idx = HashKey(ctx, k);

    // Linear probing
    SMNode* data = Resolve(ctx, m_data);
    for (u32 i = 0; i < MaxChainLength; i++) {
      if (data[idx].InUse && EqualToKey(ctx, data[idx].Node.Key, k)) return &data[idx].Node.Value;
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
    SMNode* data = Resolve(ctx, m_data);

    AssignT assigner{ctx};
    assigner(data[index].Node.Key, k);

    data[index].Node.Value = std::move(v);
    data[index].InUse = true;
    m_size += 1;

    return &data[index].Node;
  }

  /// Remove the key `k`
  bool Remove(Context* ctx, const KeyT& k) {
    i32 idx = HashKey(ctx, k);

    // Linear probing
    SMNode* data = Resolve(ctx, m_data);
    for (u32 i = 0; i < MaxChainLength; i++) {
      if (data[idx].InUse && EqualToKey(ctx, data[idx].Node.Key, k)) {
        data[idx].Destruct(&ctx->Memory());
        data[idx].InUse = false;
        --m_size;
        return true;
      }
      idx = (idx + 1) % m_capacity;
    }
    return false;
  }

  /// Get the size of the map
  u32 Size() const { return m_size; }

  /// Get the capacity of the map
  u32 Capacity() const { return m_capacity; }

  /// Clear the map
  void Clear(Context* ctx) {
    Destruct(&ctx->Memory());
    m_capacity = 0;
    m_size = 0;
  }

 private:
  i32 Hash(Context* ctx, const KeyT& key) const {
    if (m_size >= (m_capacity / 2)) return Invalid;

    SMNode* data = Resolve(ctx, m_data);

    // Find the best index
    i32 idx = HashKey(ctx, key);

    // Linear probing
    for (u32 i = 0; i < MaxChainLength; i++) {
      if (!data[idx].InUse) return idx;
      if (data[idx].InUse && EqualToKey(ctx, data[idx].Node.Key, key)) return idx;
      idx = (idx + 1) % m_capacity;
    }

    return Invalid;
  }

  i32 HashKey(Context* ctx, const KeyT& key) const {
    HasherT hasher{ctx};
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

  bool EqualToKey(Context* ctx, const KeyT& key1, const KeyT& key2) const {
    EqualToT equal{ctx};
    return equal(key1, key2);
  }

  void Rehash(Context* ctx) {
    // Update the array
    auto oldData = m_data;
    u32 oldCapacity = m_capacity;

    // Update the size
    m_capacity = std::max((u32)4, 2 * m_capacity);
    m_size = 0;
    m_data = NewArray<SMNode>(ctx, 2 * m_capacity);

    // Rehash the elements
    if (!oldData.IsNull()) {
      SMNode* oldDataP = Resolve(ctx, oldData);
      for (u32 i = 0; i < oldCapacity; i++) {
        if (!oldDataP[i].InUse) continue;
        Insert(ctx, oldDataP[i].Node.Key, std::move(oldDataP[i].Node.Value));
      }
      DeleteArray(ctx, oldData, oldCapacity);
    }
  }

 private:
  Ptr<SMNode> m_data;
  u32 m_capacity;
  u32 m_size;
};

}  // namespace bifrost