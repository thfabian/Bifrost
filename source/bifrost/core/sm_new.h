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
#include "bifrost/core/sm_object.h"
#include "bifrost/core/ptr.h"

namespace bifrost {

namespace internal {

template <class T>
std::enable_if_t<std::is_base_of<SMObject, T>::value> Destruct(SharedMemory* mem, T* ptr) {
  ptr->Destruct(mem);
  ptr->~T();
}

template <class T>
std::enable_if_t<!std::is_base_of<SMObject, T>::value> Destruct(SharedMemory* mem, T* ptr) {
  ptr->~T();
}

}  // namespace internal

/// Create a new array of type ``T`` and length ``len`` and return a shared memory pointer to it
template <class T, class... ArgsT>
inline Ptr<T> NewArray(SharedMemory* mem, u64 len, ArgsT&&... args) {
  auto ptr = static_cast<T*>(mem->Allocate(sizeof(T) * len));
  if (!ptr) throw std::bad_alloc();

  for (u64 i = 0; i < len; ++i) {
    ::new (ptr + i) T(std::forward<ArgsT>(args)...);
  }
  return Ptr<T>(mem->Offset(static_cast<void*>(ptr)));
}
template <class T, class... ArgsT>
inline Ptr<T> NewArray(Context* ctx, u64 len, ArgsT&&... args) {
  return NewArray<T>(&ctx->Memory(), len, std::forward<ArgsT>(args)...);
}

/// Create a new object of type ``T`` constructing it with ``args``
template <class T, class... ArgsT>
inline Ptr<T> New(SharedMemory* mem, ArgsT&&... args) {
  return NewArray<T>(mem, 1, std::forward<ArgsT>(args)...);
}
template <class T, class... ArgsT>
inline Ptr<T> New(Context* ctx, ArgsT&&... args) {
  return New<T>(&ctx->Memory(), std::forward<ArgsT>(args)...);
}

/// Delete array of length ``len`` given by ``ptr``
template <class T>
inline void DeleteArray(Context* ctx, Ptr<T> ptr, u64 len) {
  DeleteArray(&ctx->Memory(), ptr, len);
}
template <class T>
inline void DeleteArray(SharedMemory* mem, Ptr<T> ptr, u64 len) {
  if (len == 0) return;

  T* ptrV = ptr.Resolve((void*)mem->GetBaseAddress());
  for (u64 i = 0; i < len; ++i) {
    internal::Destruct(mem, ptrV + i);
  }
  mem->Deallocate((void*)ptrV);
}

/// Delete pointer ``ptr``
template <class T>
inline void Delete(Context* ctx, Ptr<T> ptr) {
  DeleteArray(&ctx->Memory(), ptr, 1);
}
template <class T>
inline void Delete(SharedMemory* mem, Ptr<T> ptr) {
  DeleteArray(mem, ptr, 1);
}

}  // namespace bifrost
