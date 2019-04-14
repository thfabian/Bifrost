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

#include "bifrost/shared/bifrost_shared.h"
#include "bifrost/shared/common.h"

namespace bifrost::shared {

/// Pointer represented as an offset from a base address
template <class T>
class Ptr {
 public:
  inline Ptr() : m_offsetInBytes(0) {}

  inline Ptr(const Ptr<T>& other) = default;
  inline Ptr(Ptr<T>&&) = default;

  inline Ptr<T>& operator=(const Ptr<T>& other) = default;
  inline Ptr<T>& operator=(Ptr<T>&&) = default;

  /// Construct from `addr` (computes byte offset `ptr - base_ptr`)
  static Ptr<T> FromAddress(const void* ptr, const void* base_ptr) noexcept { return Ptr<T>((u64)ptr - (u64)base_ptr); }

  /// Assign offset
  explicit inline Ptr(u64 offset) : m_offsetInBytes(offset) {}
  inline Ptr<T>& operator=(u64 offset) {
    m_offsetInBytes = offset;
    return *this;
  }

  /// Cast to `U`
  template <class U>
  inline Ptr<U> Cast() const noexcept {
    return Ptr<U>(m_offsetInBytes);
  }

  /// Resolve the offset to recover the pointer
  inline T* Resolve(void* base_address) noexcept { return (T*)(u64(base_address) + m_offsetInBytes); }
  inline const T* Resolve(void* base_address) const noexcept { return (T*)(u64(base_address) + m_offsetInBytes); }

  /// Get the offset in bytes
  inline u64 Offset() const noexcept { return m_offsetInBytes; }

  /// Addition and assignment
  inline Ptr<T>& operator+=(u64 i) noexcept {
    m_offsetInBytes += sizeof(T) * i;
    return *this;
  }

  /// Subtraction and assignment
  inline Ptr<T>& operator-=(u64 i) noexcept {
    m_offsetInBytes -= sizeof(T) * i;
    return *this;
  }

  /// Comparisons
  inline bool operator<(const Ptr<T>& rhs) const noexcept { return m_offsetInBytes < rhs.Offset(); }
  inline bool operator<=(const Ptr<T>& rhs) const noexcept { return m_offsetInBytes <= rhs.Offset(); }
  inline bool operator==(const Ptr<T>& rhs) const noexcept { return m_offsetInBytes == rhs.Offset(); }
  inline bool operator!=(const Ptr<T>& rhs) const noexcept { return m_offsetInBytes != rhs.Offset(); }
  inline bool operator>=(const Ptr<T>& rhs) const noexcept { return m_offsetInBytes >= rhs.Offset(); }
  inline bool operator>(const Ptr<T>& rhs) const noexcept { return m_offsetInBytes > rhs.Offset(); }

  /// Addition
  friend inline Ptr<T> operator+(const Ptr<T>& lhs, u64 i) noexcept { return Ptr<T>(lhs.Offset() + sizeof(T) * i); }
  friend inline Ptr<T> operator+(u64 i, const Ptr<T>& rhs) noexcept { return rhs + i; }

  /// Subtraction
  friend inline Ptr<T> operator-(const Ptr<T>& lhs, u64 i) noexcept { return Ptr<T>(lhs.Offset() - sizeof(T) * i); }
  friend inline Ptr<T> operator-(u64 i, const Ptr<T>& rhs) noexcept { return rhs - i; }

  /// Convert to stream
  friend std::ostream& operator<<(std::ostream& os, const Ptr<T>& ptr) { return internal::StreamOffset(os, ptr.Offset()); }

 private:
  u64 m_offsetInBytes;
};

namespace internal {

BIFROST_SHARED_API std::ostream& StreamOffset(std::ostream& os, u64 offset) noexcept;

}

}  // namespace bifrost::shared
