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

#include "bifrost/core/object.h"
#include "bifrost/core/ptr.h"

namespace bifrost {

/// Create a new array of type ``T`` and length ``len`` and return a shared memory pointer to it
template <class T, class... ArgsT>
inline Ptr<T> NewArray(ISharedMemory* mem, u64 len, ArgsT&&... args) {
  auto ptr = static_cast<T*>(mem->Allocate(sizeof(T) * len));
  if (!ptr) throw std::bad_alloc();

  for (u64 i = 0; i < len; ++i) {
    ::new (ptr + i) T(std::forward<ArgsT>(args)...);
  }
  return Ptr<T>(mem->Offset(static_cast<void*>(ptr)));
}
template <class T, class... ArgsT>
inline Ptr<T> NewArray(Context* ctx, u64 len, ArgsT&&... args) {
  return NewArray<T>(&ctx->SharedMemory(), len, std::forward<ArgsT>(args)...);
};
template <class T, class... ArgsT>
inline Ptr<T> New(Object* obj, u64 len, ArgsT&&... args) {
  return NewArray<T>(&obj->GetContext(), len, std::forward<ArgsT>(args)...);
}

/// Create a new object of type ``T`` constructing it with ``args``
template <class T, class... ArgsT>
inline Ptr<T> New(ISharedMemory* mem, ArgsT&&... args) {
  return NewArray<T>(mem, 1, std::forward<ArgsT>(args)...);
}
template <class T, class... ArgsT>
inline Ptr<T> New(Context* ctx, ArgsT&&... args) {
  return New<T>(&ctx->SharedMemory(), std::forward<ArgsT>(args)...);
}
template <class T, class... ArgsT>
inline Ptr<T> New(Object* obj, ArgsT&&... args) {
  return New<T>(&obj->GetContext(), std::forward<ArgsT>(args)...);
}

/// Delete pointer ``ptr``
template <class T>
inline void Delete(ISharedMemory* mem, Ptr<T> ptr) {
  DeleteArray(mem, ptr, 1);
}
template <class T>
inline void Delete(Context* mem, Ptr<T> ptr) {
  Delete(&ctx->SharedMemory(), ptr);
}
template <class T>
inline void Delete(Object* obj, Ptr<T> ptr) {
  Delete(&obj->SharedMemory(), ptr);
}

/// Delete array of length ``len`` given by ``ptr``
template <class T>
inline void DeleteArray(ISharedMemory* mem, Ptr<T> ptr, u64 len) {
  for (u64 i = 0; i < len; ++i) {
    auto ptrV = ptr->Resolve(mem->GetBaseAddress());
    ptrV->~T();
    mem->Deallocate(ptrV);
  }
}
template <class T>
inline void DeleteArray(Context* mem, Ptr<T> ptr, u64 len) {
  DeleteArray(&ctx->SharedMemory(), ptr, len);
}
template <class T>
inline void DeleteArray(Object* obj, Ptr<T> ptr, u64 len) {
  DeleteArray(&obj->SharedMemory(), ptr, len);
}

}  // namespace bifrost
