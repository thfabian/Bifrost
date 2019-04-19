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
#include "bifrost/core/new.h"

namespace bifrost {

/// Shared memory string
template <class ValueT>
class SMList final : public Object {
 public:
  struct Node {
    ValueT Value;
    Ptr<Node> Prev;
    Ptr<Node> Next;
  };

  /// Construct an empty list
  SMList(Context* ctx) : Object(ctx) {}

  /// Insert a new node with value `v` after node `n`
  void Insert(Ptr<FreeListNode> node, ValueT v) {
    // auto node = New<Node>(this);
    // auto nodeP = Resolve(node);
    // nodeP->Value = std::move(v);

    // if (Empty()) {
    //  m_head = node;
    //  m_tail = node;
    //} else {
    //  if (m_head == m_tail)
    //  {

    //  }

    //  auto headP = Resolve(m_head);
    //  headP->Next = node;
    //  nodeP->Prev = m_head;
    //  m_head = node;

    //}
  }

 private:
  Ptr<Node> m_head;
  Ptr<Node> m_tail;
};

}  // namespace bifrost
