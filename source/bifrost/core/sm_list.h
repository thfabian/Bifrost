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

/// Shared memory string
template <class ValueT>
class SMList : public SMObject {
 public:
  struct Node {
    ValueT Value;
    Ptr<Node> Prev = Ptr<Node>();
    Ptr<Node> Next = Ptr<Node>();
  };

  /// Destruct the list
  void Destruct(SharedMemory* mem) {
    while (!Empty()) {
      Ptr<Node> tailA = m_tail;
      Node* tailP = Resolve(mem, tailA);

      m_tail = tailP->Next;
      if (!m_tail.IsNull()) Resolve(mem, m_tail)->Prev = Ptr<Node>();

      Delete(mem, tailA);
    }
  }

  /// Create a new node with value `v` and treat it as the new head
  void PushFront(Context* ctx, ValueT v) {
    Ptr<Node> oldHead = m_head;

    m_head = New<Node>(ctx);
    Resolve(ctx, m_head)->Value = std::move(v);
    Resolve(ctx, m_head)->Prev = oldHead;

    if (!oldHead.IsNull()) Resolve(ctx, oldHead)->Next = m_head;
    if (m_tail.IsNull()) m_tail = m_head;
  }

  /// Return the value of head or NULL if list is empty
  ValueT* PeekFront(Context* ctx) const {
    if (m_head.IsNull()) return nullptr;
    return &Resolve(ctx, m_head)->Value;
  }

  /// Insert a new node with value `v` *after* node `pos`
  void Insert(Context* ctx, Node* pos, ValueT v) {
    if (Empty()) {
      PushFront(ctx, std::move(v))
    }
    if (pos == Resolve(ctx, m_tail)) {
      PushBack(ctx, std::move(v));
    } else {
      Ptr<Node> posA = Ptr<Node>::FromAddress(pos, ctx->Memory().GetBaseAddress());
      Ptr<Node> nodeA = New<Node>(ctx);

      Node* nodeP = Resolve(ctx, nodeA);
      Node* posP = pos;

      nodeP->Value = std::move(v);

      Ptr<Node> posPrevA = posP->Prev;
      Node* posPrevP = Resolve(ctx, posPrevA);

      nodeP->Prev = posPrevA;
      nodeP->Next = posA;
      posP->Prev = nodeA;
      posPrevP->Next = nodeA;
    }
  }

  /// Create a new node with value `v` and treat it as the new tail
  void PushBack(Context* ctx, ValueT v) {
    Ptr<Node> oldTail = m_tail;

    m_tail = New<Node>(ctx);
    Resolve(ctx, m_tail)->Value = std::move(v);
    Resolve(ctx, m_tail)->Next = oldTail;

    if (!oldTail.IsNull()) Resolve(ctx, oldTail)->Prev = m_tail;
    if (m_head.IsNull()) m_head = m_tail;
  }

  /// Return the value of tail or NULL if list is empty
  ValueT* PeekBack(Context* ctx) const {
    if (m_tail.IsNull()) return nullptr;
    return &Resolve(ctx, m_tail)->Value;
  }

  /// Erase the node `pos`
  inline void Erase(Context* ctx, Node* pos, bool deferDelete = false) {
    Node* posP = pos;
    Ptr<Node> posA = Ptr<Node>::FromAddress(posP, ctx->Memory().GetBaseAddress());

    if (posA == m_head) {
      if (m_head == m_tail) {
        m_head = Ptr<Node>();
        m_tail = Ptr<Node>();
      } else {
        Ptr<Node> oldPrev = posP->Prev;
        m_head = oldPrev;
        Resolve(ctx, m_head)->Next = Ptr<Node>();
      }
    } else if (posA == m_tail) {
      Ptr<Node> oldNext = posP->Next;
      m_tail = oldNext;
      Resolve(ctx, m_tail)->Prev = Ptr<Node>();
    } else {
      Ptr<Node> oldPrev = posP->Prev;
      Ptr<Node> oldNext = posP->Next;
      Resolve(ctx, oldPrev)->Next = oldNext;
      Resolve(ctx, oldNext)->Prev = oldPrev;
    }

    if (!deferDelete) {
      Delete(ctx, posA);
    }
  }

  /// Remove the tail
  void PopBack(Context* ctx) { Erase(ctx, Resolve(ctx, GetTail())); }

  /// Remove the head
  void PopFront(Context* ctx, bool deferDelete = false) { Erase(ctx, Resolve(ctx, GetHead()), deferDelete); }

  /// Iterate from head to tail
  ///
  /// Return `false` to stop iteration, `true` to continue
  template <class FunctorT>
  inline void ForeachHeadToTail(Context* ctx, FunctorT&& functor) const {
    for (Ptr<Node> curNode = m_head; !curNode.IsNull(); curNode = Resolve(ctx, curNode)->Prev) {
      if (!functor(Resolve(ctx, curNode))) break;
    }
  }

  /// Iterate from head to tail
  ///
  /// Return `false` to stop iteration, `true` to continue
  template <class FunctorT>
  inline void ForeachTailToHead(Context* ctx, FunctorT&& functor) const {
    for (Ptr<Node> curNode = m_tail; !curNode.IsNull(); curNode = Resolve(ctx, curNode)->Next) {
      if (!functor(Resolve(ctx, curNode))) break;
    }
  }

  /// Get the size of the list
  inline u64 Size(Context* ctx) const {
    u64 size = 0;
    ForeachHeadToTail(ctx, [&size](Node* node) -> bool {
      size += 1;
      return true;
    });
    return size;
  }

  /// Check if list is empty
  bool Empty() const { return m_head.IsNull() || m_tail.IsNull(); }

  /// Get the head pointer
  Ptr<Node> GetHead() const { return m_head; }

  /// Get the head pointer
  Ptr<Node> GetTail() const { return m_tail; }

 private:
  Ptr<Node> m_head = Ptr<Node>();
  Ptr<Node> m_tail = Ptr<Node>();
};

}  // namespace bifrost
