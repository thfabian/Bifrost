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

#include "bifrost_shared/common.h"
#include "bifrost_shared/shared_object.h"
#include "bifrost_shared/shared_memory.h"

namespace bifrost::shared {

/// STL compatible allocator
template <class T>
class SharedAllocator {
 private:
 public:
  // Typedefs
  typedef T value_type;
  typedef value_type* pointer;
  typedef const value_type* const_pointer;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;

  inline SharedAllocator() = default;
  inline SharedAllocator(const SharedAllocator<T>& other) = default;
  inline SharedAllocator(SharedAllocator<T>&& other) = default;

  // Convert an allocator<T> to allocator<U>
  template <typename U>
  struct rebind {
    typedef SharedAllocator<U> other;
  };
  template <typename U>
  inline explicit SharedAllocator(const SharedAllocator<U>& other) {}

  // Address
  inline pointer address(reference r) { return &r; }
  inline const_pointer address(const_reference r) { return &r; }

  // Memory allocation
  inline pointer allocate(size_type cnt) { return (pointer)SharedObject::Get().GetSharedMemory()->Allocate(u64(cnt) * sizeof(T)); }
  inline void deallocate(pointer p, size_type) { SharedObject::Get().GetSharedMemory()->Deallocate(p); }

  // Size
  inline size_type max_size() const { return SharedObject::Get().GetSharedMemory()->GetSizeInBytes(); }

  // Construction/destruction
  template <class... Args>
  inline void construct(pointer p, Args&&... args) {
    ::new (p) T(std::forward<Args>(args)...);
  }
  inline void destroy(pointer p) { p->~T(); }

  inline bool operator==(SharedAllocator<T> const& other) { return true }
  inline bool operator!=(SharedAllocator<T> const& other) { return !(operator==(other)); }
};

namespace stl {

using string = std::basic_string<char, std::char_traits<char>, SharedAllocator<char>>;
using wstring = std::basic_string<wchar_t, std::char_traits<wchar_t>, SharedAllocator<wchar_t>>;

template <class T>
using vector = std::vector<T, SharedAllocator<T>>;

template <class T>
using list = std::list<T, SharedAllocator<T>>;

template <class T>
using deque = std::deque<T, SharedAllocator<T>>;

template <class K, class V>
using unordered_map = std::unordered_map<K, V, std::hash<K>, std::equal_to<K>, SharedAllocator<std::pair<const K, V>>>;

}  // namespace stl

}  // namespace bifrost::shared