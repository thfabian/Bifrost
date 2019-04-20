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
class SMString : public SMObject {
 public:
  SMString(Context* ctx, u32 initialSize = 0) : m_size(0) { Allocate(ctx, initialSize); }
  SMString(Context* ctx, const std::string& s) : SMString(ctx, std::string_view{s}) {}
  SMString(Context* ctx, const char* s) : SMString(ctx, std::string_view{s}) {}
  SMString(Context* ctx, std::string_view s) : m_size(0) { Assign(ctx, s); }

  /// Destructor
  void Destruct(SharedMemory* mem) { Deallocate(mem); }

  /// Move constructor
  SMString(SMString&& s) { Move(std::move(s)); }
  SMString& operator=(SMString&& s) {
    Move(std::move(s));
    return *this;
  }

  /// Get a view of the data
  std::string_view AsView(Context* ctx) const {
    char* data = Resolve(ctx, m_data);
    return std::string_view{data, m_size};
  }

  /// Get a copy of the data
  std::string AsString(Context* ctx) const {
    char* data = Resolve(ctx, m_data);
    return std::string{data, m_size};
  }

  /// Length of the string
  u32 Size() const { return m_size; }

  /// Assign the string view
  void Assign(Context* ctx, std::string_view s) {
    if (s.size() > m_size) {
      Deallocate(&ctx->Memory());
      Allocate(ctx, static_cast<u32>(s.size()));
    }
    Copy(ctx, s);
  }
  void Assign(Context* ctx, const SMString& s) { Assign(ctx, s.AsView(ctx)); }

 private:
  void Move(SMString&& s) {
    m_data = s.m_data;
    m_size = s.m_size;
    s.m_size = 0;
  }

  void Copy(Context* ctx, std::string_view s) {
    BIFROST_ASSERT(m_size >= s.size());
    char* data = Resolve(ctx, m_data);
    std::memcpy(data, s.data(), s.size());
    m_size = static_cast<u32>(s.size());
  }

  void Deallocate(SharedMemory* mem) {
    if (m_size != 0) {
      DeleteArray(mem, m_data, m_size);
      m_size = 0;
    }
  }

  void Allocate(Context* ctx, u32 size) {
    if (size > 0) {
      m_data = NewArray<char>(ctx, size);
    }
    m_size = size;
  }

 private:
  Ptr<char> m_data;
  u32 m_size;
};

}  // namespace bifrost
