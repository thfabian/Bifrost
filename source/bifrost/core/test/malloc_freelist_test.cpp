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

#include "bifrost/core/test/test.h"
#include "bifrost/core/malloc_freelist.h"

namespace {

using namespace bifrost;

static void Dump(FreeList& free_list, void* base_addr) noexcept {
  u32 index = 0;
  free_list.ForeachHeadToTail(
      [&index, base_addr](Ptr<FreeListNode> node) -> bool {
        AllocNode* n = (AllocNode*)node.Resolve(base_addr);
        std::cout << "[Node " << index << "] " << (node.IsNull() ? "NULL" : std::to_string(node.Offset())) << "\n";
        std::cout << "  Next = " << (n->Node.Next.IsNull() ? "NULL" : std::to_string(n->Node.Next.Offset())) << "\n";
        std::cout << "  Prev = " << (n->Node.Prev.IsNull() ? "NULL" : std::to_string(n->Node.Prev.Offset())) << "\n";
        std::cout << "  Size = " << n->Size << "\n";
        index += 1;
        return true;
      },
      base_addr);
}

TEST(MallocFreelistTest, Allocate128) {
  const u64 num_bytes = 1024;
  const u64 block_size = MallocFreeList::BlockSize;
  byte* start_address = (byte*)_aligned_malloc(num_bytes, MallocFreeList::BlockSize);

  MallocFreeList* freelist = MallocFreeList::Create(start_address, num_bytes);
  ASSERT_EQ((u64)freelist - (u64)start_address, *((u64*)start_address));

  ASSERT_EQ(1, freelist->GetFreeList().Size(start_address));
  u64 free_bytes_after_construction = freelist->GetNumFreeBytes(start_address);

  EXPECT_EQ(num_bytes - (block_size /* this pointer offset */ + sizeof(MallocFreeList) + sizeof(AllocNode)), free_bytes_after_construction);

  // Allocate 128 bytes -> split the initial free block
  //
  //  [MemBlock0][  128  ][MemBlock1][ ... ]
  //  ^~~~~~~~~~~~~~~~~~~^
  //              |
  //             192 = 128 + sizeof(AllocNode)=64
  void* ptr = freelist->Allocate(128, start_address);
  ASSERT_EQ(1, freelist->GetFreeList().Size(start_address));

  AllocNode* block_0 = (AllocNode*)((u64)ptr - sizeof(AllocNode));
  EXPECT_EQ(Ptr<FreeListNode>(), block_0->Node.Next);
  EXPECT_EQ(Ptr<FreeListNode>(), block_0->Node.Prev);
  EXPECT_EQ(128, block_0->Size);
  EXPECT_EQ(free_bytes_after_construction - block_0->Size - sizeof(AllocNode), freelist->GetNumFreeBytes(start_address));

  AllocNode* block_1 = (AllocNode*)((u64)ptr + block_0->Size);
  ASSERT_EQ((u64)freelist->GetFreeList().GetHead().Resolve(start_address), (u64)block_1);
  EXPECT_EQ(Ptr<FreeListNode>(), block_1->Node.Next);
  EXPECT_EQ(Ptr<FreeListNode>(), block_1->Node.Prev);
  EXPECT_EQ(freelist->GetNumFreeBytes(start_address), block_1->Size);

  // Free the memory again (defragmentation should restore the state after initial allocation)
  freelist->Deallocate(ptr, start_address);
  ASSERT_EQ(1, freelist->GetFreeList().Size(start_address));
  EXPECT_EQ(free_bytes_after_construction, freelist->GetNumFreeBytes(start_address));

  _aligned_free(start_address);
}

TEST(MallocFreelistTest, AllocateMax) {
  const u64 num_bytes = 1024;
  const u64 block_size = MallocFreeList::BlockSize;
  byte* start_address = (byte*)_aligned_malloc(num_bytes, MallocFreeList::BlockSize);

  MallocFreeList* freelist = MallocFreeList::Create(start_address, num_bytes);
  ASSERT_EQ((u64)freelist - (u64)start_address, *((u64*)start_address));
  ASSERT_EQ(1, freelist->GetFreeList().Size(start_address));
  u64 free_bytes_after_construction = freelist->GetNumFreeBytes(start_address);

  // Allocate all memory
  void* ptr = freelist->Allocate(free_bytes_after_construction, start_address);
  ASSERT_EQ(0, freelist->GetFreeList().Size(start_address));

  // Allocate again
  EXPECT_EQ(nullptr, freelist->Allocate(free_bytes_after_construction, start_address));

  freelist->Deallocate(ptr, start_address);
  EXPECT_EQ(free_bytes_after_construction, freelist->GetNumFreeBytes(start_address));

  _aligned_free(start_address);
}

TEST(MallocFreelistTest, Misaligned) {
  const u64 num_bytes = 1024;
  const u64 block_size = MallocFreeList::BlockSize;
  byte* allocated_start_address1 = (byte*)_aligned_malloc(num_bytes, MallocFreeList::BlockSize);
  byte* allocated_start_address2 = (byte*)_aligned_malloc(num_bytes + 1, MallocFreeList::BlockSize);
  byte* allocated_start_address3 = (byte*)_aligned_malloc(num_bytes + (block_size - 1), MallocFreeList::BlockSize);

  // Aligned
  byte* start_address1 = allocated_start_address1;
  MallocFreeList* freelist1 = MallocFreeList::Create(start_address1, num_bytes);

  // Off by 1 (still okay as we have 56 bytes spare in the this pointer block)
  byte* start_address2 = allocated_start_address2 + 1;
  MallocFreeList* freelist2 = MallocFreeList::Create(start_address2, num_bytes);

  // Off by block_size - 1 (won't fit in the first block)
  byte* start_address3 = allocated_start_address3 + (block_size - 1);
  MallocFreeList* freelist3 = MallocFreeList::Create(start_address3, num_bytes);

  ASSERT_EQ((u64)freelist1 - (u64)start_address1, *((u64*)start_address1));
  ASSERT_EQ((u64)freelist2 - (u64)start_address2, *((u64*)start_address2));
  ASSERT_EQ((u64)freelist3 - (u64)start_address3, *((u64*)start_address3));

  ASSERT_EQ(1, freelist1->GetFreeList().Size(start_address1));
  ASSERT_EQ(1, freelist2->GetFreeList().Size(start_address2));
  ASSERT_EQ(1, freelist3->GetFreeList().Size(start_address3));

  // Initial alignment should pad correctly
  EXPECT_EQ(freelist1->GetNumFreeBytes(start_address1), freelist2->GetNumFreeBytes(start_address2));
  EXPECT_EQ(freelist1->GetNumFreeBytes(start_address1) - block_size, freelist3->GetNumFreeBytes(start_address3));

  _aligned_free(allocated_start_address1);
  _aligned_free(allocated_start_address2);
  _aligned_free(allocated_start_address3);
}

TEST(MallocFreelistTest, Defragmentation) {
  const u64 num_bytes = 1024;
  const u64 block_size = MallocFreeList::BlockSize;
  byte* start_address = (byte*)_aligned_malloc(num_bytes, MallocFreeList::BlockSize);

  MallocFreeList* freelist = MallocFreeList::Create(start_address, num_bytes);
  ASSERT_EQ((u64)freelist - (u64)start_address, *((u64*)start_address));
  ASSERT_EQ(1, freelist->GetFreeList().Size(start_address));
  u64 free_bytes_after_construction = freelist->GetNumFreeBytes(start_address);
  u64 num_blocks = free_bytes_after_construction / (block_size + sizeof(AllocNode));

  std::vector<void*> ptrs(num_blocks, nullptr);
  for (auto& ptr : ptrs) {
    ptr = freelist->Allocate(block_size, start_address);
    EXPECT_NE(nullptr, ptr);

    AllocNode* alloc_node = (AllocNode*)((u64)ptr - sizeof(AllocNode));
    EXPECT_EQ(block_size, alloc_node->Size);
  }


  ASSERT_EQ(1, freelist->GetFreeList().Size(start_address));
  EXPECT_EQ(0, ((AllocNode*)freelist->GetFreeList().GetHead().Resolve(start_address))->Size);
  EXPECT_EQ(0, freelist->GetNumFreeBytes(start_address));

  freelist->Deallocate(ptrs[0], start_address);
  EXPECT_EQ(block_size, freelist->GetNumFreeBytes(start_address));
  EXPECT_EQ(2, freelist->GetFreeList().Size(start_address));

  // Merge with right
  freelist->Deallocate(ptrs[1], start_address);
  EXPECT_EQ(2 * block_size + sizeof(AllocNode), freelist->GetNumFreeBytes(start_address));
  EXPECT_EQ(2, freelist->GetFreeList().Size(start_address));

  freelist->Deallocate(ptrs[4], start_address);
  EXPECT_EQ(3 * block_size + sizeof(AllocNode), freelist->GetNumFreeBytes(start_address));
  EXPECT_EQ(3, freelist->GetFreeList().Size(start_address));

  // Merge with left
  freelist->Deallocate(ptrs[3], start_address);
  EXPECT_EQ(4 * block_size + 2 * sizeof(AllocNode), freelist->GetNumFreeBytes(start_address));
  EXPECT_EQ(3, freelist->GetFreeList().Size(start_address));

  // Reclaim the very last block and merge with the trailing AllocNode
  freelist->Deallocate(ptrs[5], start_address);
  EXPECT_EQ(7 * block_size + 2 * sizeof(AllocNode), freelist->GetNumFreeBytes(start_address));
  EXPECT_EQ(2, freelist->GetFreeList().Size(start_address));

  // Merge everything to one block
  freelist->Deallocate(ptrs[2], start_address);
  EXPECT_EQ(free_bytes_after_construction, freelist->GetNumFreeBytes(start_address));
  EXPECT_EQ(1, freelist->GetFreeList().Size(start_address));

  _aligned_free(start_address);
}

}  // namespace