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

#include "bifrost_core/common.h"
#include "bifrost_core/allocation.h"

namespace bifrost {

Allocation::Memory Allocation::TryStackMalloc(std::size_t sizeInBytes) {
  void* ptr = nullptr;
  bool heapAllocated = false;

  // Try stack allocation
  if (sizeInBytes < Allocation::MaxStackAllocationSizeInBytes) {
    ptr = StackMalloc(sizeInBytes);
  }

  // Revert to heap allocation
  if (!ptr) {
    ptr = std::malloc(sizeInBytes);
    heapAllocated = true;
  }
  return Allocation::Memory(ptr, heapAllocated);
}

void* Allocation::StackMalloc(std::size_t sizeInBytes) {
  __try {
    return ::_alloca(sizeInBytes);
  } __except (GetExceptionCode() == STATUS_STACK_OVERFLOW) {
    assert(::_resetstkoflw() != 0 && "Could not reset stack");
  }
  return nullptr;
}

Allocation::Memory::Memory(void* ptr, bool heapAllocated) : m_ptr(ptr), m_heapAllocated(heapAllocated) {}

Allocation::Memory::Memory(Memory&& other) {
  m_ptr = other.m_ptr;
  m_heapAllocated = other.m_heapAllocated;
  other.m_ptr = nullptr;
}

bifrost::Allocation::Memory& Allocation::Memory::operator=(Memory&& other) {
  m_ptr = other.m_ptr;
  m_heapAllocated = other.m_heapAllocated;
  other.m_ptr = nullptr;
  return *this;
}

Allocation::Memory::~Memory() {
  if (m_heapAllocated) std::free(m_ptr);
}

}  // namespace bifrost
