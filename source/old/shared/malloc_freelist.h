////   ____  _  __               _
////  |  _ \(_)/ _|             | |
////  | |_) |_| |_ _ __ ___  ___| |_
////  |  _ <| |  _| '__/ _ \/ __| __|
////  | |_) | | | | | | (_) \__ \ |_
////  |____/|_|_| |_|  \___/|___/\__|   2018 - 2019
////
////
//// This file is distributed under the MIT License (MIT).
//// See LICENSE.txt for details.
//
//#pragma once
//
//#include "bifrost_shared/common.h"
//#include "bifrost_shared/ptr.h"
//#include "bifrost_shared/config.h"
//#include "bifrost_shared/bifrost_shared.h"
//#include "bifrost/core/padding.h"
//#include "bifrost/core/mutex.h"
//
//namespace bifrost::shared {
//
//#pragma pack(push)
//#pragma pack(1)
//
//struct FreeListNode {
//  Ptr<FreeListNode> Next = Ptr<FreeListNode>();
//  Ptr<FreeListNode> Prev = Ptr<FreeListNode>();
//};
//
//struct AllocNode {
//  FreeListNode Node;
//  u64 Size;
//  Padding<BIFROST_MALLOC_FREELIST_BLOCKSIZE - (sizeof(FreeListNode) + sizeof(u64))> Pad;
//};
//
//class FreeList {
// public:
//  FreeList(Ptr<FreeListNode> node) {
//    m_head = node;
//    m_tail = node;
//  }
//
//  /// Make `node` the new head
//  inline void PushFront(Ptr<FreeListNode> node, void* baseAddr) {
//    Ptr<FreeListNode> oldHead = m_head;
//
//    m_head = node;
//    m_head.Resolve(baseAddr)->Prev = oldHead;
//    if (oldHead.Offset() != 0) oldHead.Resolve(baseAddr)->Next = m_head;
//    if (m_tail.Offset() == 0) m_tail = m_head;
//  }
//
//  /// Make `node` the new tail
//  inline void PushBack(Ptr<FreeListNode> node, void* baseAddr) {
//    Ptr<FreeListNode> old_tail = m_tail;
//
//    m_tail = node;
//    m_tail.Resolve(baseAddr)->Next = old_tail;
//    if (old_tail.Offset() != 0) old_tail.Resolve(baseAddr)->Prev = m_tail;
//    if (m_head.Offset() == 0) m_head = m_tail;
//  }
//
//  /// Insert `node` after pos
//  inline void Insert(Ptr<FreeListNode> pos, Ptr<FreeListNode> node, void* baseAddr) {
//    if (pos == m_tail) {
//      PushBack(node, baseAddr);
//    } else {
//      FreeListNode* noder = node.Resolve(baseAddr);
//      FreeListNode* posr = pos.Resolve(baseAddr);
//
//      Ptr<FreeListNode> posPrev = posr->Prev;
//      FreeListNode* posPrevr = posPrev.Resolve(baseAddr);
//
//      noder->Prev = posPrev;
//      noder->Next = pos;
//      posr->Prev = node;
//      posPrevr->Next = node;
//    }
//  }
//
//  /// Erase the node
//  inline void Erase(Ptr<FreeListNode> node, void* baseAddr) {
//    FreeListNode* noder = node.Resolve(baseAddr);
//
//    if (node == m_head) {
//      if (m_head == m_tail) {
//        m_head = Ptr<FreeListNode>();
//        m_tail = Ptr<FreeListNode>();
//      } else {
//        Ptr<FreeListNode> oldPrev = noder->Prev;
//        m_head = oldPrev;
//        m_head.Resolve(baseAddr)->Next = Ptr<FreeListNode>();
//      }
//    } else if (node == m_tail) {
//      Ptr<FreeListNode> oldNext = noder->Next;
//      m_tail = oldNext;
//      m_tail.Resolve(baseAddr)->Prev = Ptr<FreeListNode>();
//    } else {
//      Ptr<FreeListNode> oldPrev = noder->Prev;
//      Ptr<FreeListNode> oldNext = noder->Next;
//      oldPrev.Resolve(baseAddr)->Next = oldNext;
//      oldNext.Resolve(baseAddr)->Prev = oldPrev;
//    }
//
//    noder->Next = Ptr<FreeListNode>();
//    noder->Prev = Ptr<FreeListNode>();
//  }
//
//  inline Ptr<FreeListNode> GetHead() noexcept { return m_head; }
//  inline const Ptr<FreeListNode> GetHead() const noexcept { return m_head; }
//
//  inline Ptr<FreeListNode> GetTail() noexcept { return m_tail; }
//  inline const Ptr<FreeListNode> GetTail() const noexcept { return m_tail; }
//
//  /// Iterate from head to tail
//  ///
//  /// Return `false` to stop iteration, `true` to continue
//  template <class FunctorT>
//  inline void ForeachHeadToTail(FunctorT&& functor, void* baseAddr) {
//    for (Ptr<FreeListNode> curNode = m_head; curNode != Ptr<FreeListNode>(); curNode = curNode.Resolve(baseAddr)->Prev) {
//      if (!functor(curNode)) break;
//    }
//  }
//
//  /// Iterate from head to tail
//  ///
//  /// Return `false` to stop iteration, `true` to continue
//  template <class FunctorT>
//  inline void ForeachTailToHead(FunctorT&& functor, void* baseAddr) {
//    for (Ptr<FreeListNode> curNode = m_tail; curNode != Ptr<FreeListNode>(); curNode = curNode.Resolve(baseAddr)->Next) {
//      if (!functor(curNode)) break;
//    }
//  }
//
//  /// Get the size of the list
//  inline u64 Size(void* baseAddr) noexcept {
//    u64 size = 0;
//    ForeachHeadToTail(
//        [&size](Ptr<FreeListNode> node) -> bool {
//          size += 1;
//          return true;
//        },
//        baseAddr);
//    return size;
//  }
//
// private:
//  Ptr<FreeListNode> m_head;
//  Ptr<FreeListNode> m_tail;
//};
//
///// Free list allocation strategy
//class BIFROST_SHARED_API MallocFreeList {
// public:
//  static constexpr u64 BlockSize = BIFROST_MALLOC_FREELIST_BLOCKSIZE;
//
//  /// Create a new free list allocator
//  static MallocFreeList* Create(void* startAddress, u64 numBytes);
//
//  /// Allocates a block of size bytes of memory, returning a pointer to the beginning of the block
//  void* Allocate(u64 size, void* baseAddr) noexcept;
//
//  /// Deallocates the space previously allocated with `Allocate`
//  void Deallocate(void* ptr, void* baseAddr) noexcept;
//
//  /// Get number of free bytes
//  u64 GetNumFreeBytes(void* baseAddr) const noexcept;
//
//  /// Get a pointer to the first address
//  void* GetFirstAdress(void* baseAddr) const noexcept;
//
//  /// Get the free list
//  const FreeList& GetFreeList() const noexcept;
//  FreeList& GetFreeList() noexcept;
//
// private:
//  MallocFreeList(AllocNode* block, void* baseAddr);
//
//  // BlockIt 0 ("this" pointer offset)
//
//  // BlockIt 1
//  FreeList m_list;
//  Padding<BlockSize - sizeof(FreeList)> m_pad1;
//
//  // BlockIt 2
//  mutable SpinMutex m_mutex;
//  Padding<BlockSize - sizeof(SpinMutex)> m_pad2;
//};
//
//#pragma pack(pop)
//
//}  // namespace bifrost::shared
