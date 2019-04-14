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
#include "bifrost/core/malloc_freelist.h"

namespace bifrost {

static_assert(sizeof(AllocNode) % BIFROST_MALLOC_FREELIST_BLOCKSIZE == 0, "AllocNode not aligned");

template <std::size_t Alignment>
inline u64 AlignUp(u64 addr) noexcept {
  return ((addr + (Alignment - 1)) & ~(Alignment - 1));
}

MallocFreeList* MallocFreeList::Create(void* startAddress, u64 numBytes) {
  // Make space for the offset to the "this" pointer of MallocFreeList pointer read by client and server
  byte* curStartAddress = (byte*)((u64)startAddress + sizeof(u64));
  u64 curNumBytes = numBytes - sizeof(void*);

  // Ensure start address is block aligned
  u64 addr = (u64)curStartAddress;
  u64 addrAligned = (addr + (MallocFreeList::BlockSize - 1)) & -MallocFreeList::BlockSize;
  u64 offsetFromStart = addrAligned - addr;

  curNumBytes -= offsetFromStart;
  curStartAddress += offsetFromStart;
  assert(((u64)curStartAddress & (MallocFreeList::BlockSize - 1)) == 0 && "start address of MallocFreeList not block aligned");

  curNumBytes -= curNumBytes % MallocFreeList::BlockSize;
  assert(curNumBytes % MallocFreeList::BlockSize == 0 && "invalid size");

  // Allocate space for MallocFreeList
  static_assert((sizeof(MallocFreeList) & (MallocFreeList::BlockSize - 1)) == 0, "MallocFreeList not blocked aligned");

  MallocFreeList* this_ptr = (MallocFreeList*)curStartAddress;
  u64 this_ptr_offset_from_start_in_bytes = (u64)curStartAddress - (u64)startAddress;

  std::memcpy(startAddress, &this_ptr_offset_from_start_in_bytes, sizeof(u64));
  curStartAddress += sizeof(MallocFreeList);
  curNumBytes -= sizeof(MallocFreeList);

  // Allocate first block
  assert(((u64)curStartAddress & (MallocFreeList::BlockSize - 1)) == 0 && "start address of first block not block aligned");

  AllocNode* first_block = (AllocNode*)curStartAddress;
  std::memset(first_block, 0, sizeof(AllocNode));

  first_block->Size = curNumBytes - sizeof(AllocNode);

  // Construct MallocFreeList
  ::new (this_ptr) MallocFreeList(first_block, startAddress);
  return this_ptr;
}

void* MallocFreeList::Allocate(u64 size, void* base_addr) noexcept {
  void* ptr = nullptr;

  if (size == 0) return ptr;

  BIFROST_LOCK_GUARD(m_mutex);

  // Always allocate in blocks
  size = AlignUp<BlockSize>(size);

  // Try to find a big enough block to alloc
  AllocNode* curBlock = nullptr;
  for (Ptr<FreeListNode> cur_node = m_list.GetHead(); cur_node != Ptr<FreeListNode>(); cur_node = cur_node.Resolve(base_addr)->Prev) {
    curBlock = (AllocNode*)cur_node.Resolve(base_addr);
    if (curBlock->Size >= size) {
      ptr = (void*)((u64)curBlock + sizeof(AllocNode));
      break;
    }
  }

  if (ptr) {
    // Can we split the block?
    if ((curBlock->Size - size) >= sizeof(AllocNode)) {
      AllocNode* newBlock = (AllocNode*)((u64)curBlock + sizeof(AllocNode) + size);
      newBlock->Node.Next = Ptr<FreeListNode>();
      newBlock->Node.Prev = Ptr<FreeListNode>();
      newBlock->Size = curBlock->Size - size - sizeof(AllocNode);

      curBlock->Size = size;
      m_list.Insert(Ptr<FreeListNode>::FromAddress(&curBlock->Node, base_addr), Ptr<FreeListNode>::FromAddress(&newBlock->Node, base_addr), base_addr);
    }

    m_list.Erase(Ptr<FreeListNode>::FromAddress(&curBlock->Node, base_addr), base_addr);
  }

  return ptr;
}

void MallocFreeList::Deallocate(void* ptr, void* baseAddr) noexcept {
  if (!ptr) return;

  BIFROST_LOCK_GUARD(m_mutex);

  AllocNode* block = (AllocNode*)((u64)ptr - sizeof(AllocNode));
  bool blockAdded = false;

  // Put the block back at the proper spot
  AllocNode* cur_block = nullptr;
  for (Ptr<FreeListNode> cur_node = m_list.GetHead(); cur_node != Ptr<FreeListNode>(); cur_node = cur_node.Resolve(baseAddr)->Prev) {
    cur_block = (AllocNode*)cur_node.Resolve(baseAddr);

    if (cur_block > block) {
      if (cur_node == m_list.GetHead()) {
        m_list.PushFront(Ptr<FreeListNode>::FromAddress(&block->Node, baseAddr), baseAddr);
      } else {
        m_list.Insert(cur_block->Node.Next, Ptr<FreeListNode>::FromAddress(&block->Node, baseAddr), baseAddr);
      }
      blockAdded = true;
      break;
    }
  }

  if (!blockAdded) {
    m_list.PushBack(Ptr<FreeListNode>::FromAddress(&block->Node, baseAddr), baseAddr);
  }

  // Combine adjacent blocks if possible (defragmentation)
  auto CanBeCombined = [](AllocNode* thisNode, AllocNode* otherNode) -> bool {
    if (thisNode > otherNode) {
      return ((u64)thisNode == ((u64)otherNode + sizeof(AllocNode) + otherNode->Size));
    } else {
      return ((u64)otherNode == ((u64)thisNode + sizeof(AllocNode) + thisNode->Size));
    }
  };

  bool blockAndPrev = Ptr<FreeListNode>::FromAddress(&block->Node, baseAddr) == m_list.GetTail()
                          ? false
                          : CanBeCombined((AllocNode*)block, (AllocNode*)block->Node.Prev.Resolve(baseAddr));
  bool blockAndNext = Ptr<FreeListNode>::FromAddress(&block->Node, baseAddr) == m_list.GetHead()
                          ? false
                          : CanBeCombined((AllocNode*)block, (AllocNode*)block->Node.Next.Resolve(baseAddr));

  if (blockAndNext && blockAndPrev) {
    // Combine prev, block and next -> next
    AllocNode* blockNext = (AllocNode*)block->Node.Next.Resolve(baseAddr);
    AllocNode* blockPrev = (AllocNode*)block->Node.Prev.Resolve(baseAddr);
    blockNext->Size += block->Size + blockPrev->Size + 2 * sizeof(AllocNode);
    m_list.Erase(Ptr<FreeListNode>::FromAddress(&block->Node, baseAddr), baseAddr);
    m_list.Erase(Ptr<FreeListNode>::FromAddress(&blockPrev->Node, baseAddr), baseAddr);
  } else if (blockAndNext) {
    // Combine block and next -> next
    AllocNode* blockNext = (AllocNode*)block->Node.Next.Resolve(baseAddr);
    blockNext->Size += block->Size + sizeof(AllocNode);
    m_list.Erase(Ptr<FreeListNode>::FromAddress(&block->Node, baseAddr), baseAddr);
  } else if (blockAndPrev) {
    // Combine prev and block -> block
    AllocNode* blockPrev = (AllocNode*)block->Node.Prev.Resolve(baseAddr);
    block->Size += blockPrev->Size + sizeof(AllocNode);
    m_list.Erase(Ptr<FreeListNode>::FromAddress(&blockPrev->Node, baseAddr), baseAddr);
  }
}

u64 MallocFreeList::GetNumFreeBytes(void* baseAddr) const noexcept {
  BIFROST_LOCK_GUARD(m_mutex);
  u64 freeMem = 0;
  for (Ptr<FreeListNode> curNode = m_list.GetHead(); curNode != Ptr<FreeListNode>(); curNode = curNode.Resolve(baseAddr)->Prev) {
    AllocNode* block = (AllocNode*)curNode.Resolve(baseAddr);
    freeMem += block->Size;
  }
  return freeMem;
}

void* MallocFreeList::GetFirstAdress(void* baseAddr) const noexcept { return (void*)((u64)baseAddr + BlockSize * 4); }

const FreeList& MallocFreeList::GetFreeList() const noexcept { return m_list; }
FreeList& MallocFreeList::GetFreeList() noexcept { return m_list; }

MallocFreeList::MallocFreeList(AllocNode* block, void* baseAddr) : m_list(Ptr<FreeListNode>::FromAddress(&block->Node, baseAddr)) {}

}  // namespace bifrost
