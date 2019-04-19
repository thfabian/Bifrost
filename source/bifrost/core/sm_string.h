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
#include "bifrost/core/object.h"
#include "bifrost/core/ptr.h"
#include "bifrost/core/new.h"

namespace bifrost {

/// Shared memory string
class SMString final : public Object {
 public:
  SMString(Context* ctx, u32 initialSize = 0) : Object(ctx), m_size(0) { Allocate(initialSize); }
  SMString(Context* ctx, const std::string& s) : SMString(ctx, std::string_view{s}) {}
  SMString(Context* ctx, const char* s) : SMString(ctx, std::string_view{s}) {}
  SMString(Context* ctx, std::string_view s) : Object(ctx), m_size(0) { Assign(s); }
  ~SMString() { Deallocate(); }

  /// Copy constructor
  SMString(const SMString& s) : SMString((Context*)&s.GetContext(), s.AsView()) {}

  SMString& operator=(const SMString& s) {
    Assign(s.AsView());
    return *this;
  }
  SMString& operator=(const std::string& s) { Assign(s); }
  SMString& operator=(const char* s) { Assign(s); }
  SMString& operator=(std::string_view s) { Assign(s); }

  /// Move constructor
  SMString(SMString&& s) : Object(&s.GetContext()) { Move(std::move(s)); }
  SMString& operator=(SMString&& s) {
    Move(std::move(s));
    return *this;
  }

  /// Get a view of the data
  std::string_view AsView() const {
    char* data = Resolve(m_data);
    return std::string_view{data, m_size};
  }

  /// Get a copy of the data
  std::string AsString() const {
    char* data = Resolve(m_data);
    return std::string{data, m_size};
  }

  /// Length of the string
  u32 Size() const { return m_size; }

 private:
  void Move(SMString&& s) {
    m_data = s.m_data;
    m_size = s.m_size;
    s.m_size = 0;
  }

  void Assign(std::string_view s) {
    if (s.size() > m_size) {
      Deallocate();
      Allocate(static_cast<u32>(s.size()));
    }
    Copy(s);
  }

  void Copy(std::string_view s) {
    assert(m_size >= s.size());
    char* data = Resolve(m_data);
    std::memcpy(data, s.data(), s.size());
    m_size = static_cast<u32>(s.size());
  }

  void Deallocate() {
    if (m_size != 0) {
      DeleteArray(this, m_data, m_size);
      m_size = 0;
    }
  }

  void Allocate(u32 size) {
    if (size > 0) {
      m_data = NewArray<char>(this, size);
    }
    m_size = size;
  }

 private:
  Ptr<char> m_data;
  u32 m_size;
};

/// Comparison operator
bool operator==(const SMString& s1, const SMString& s2) { return s1.AsView() == s2.AsView(); }
bool operator!=(const SMString& s1, const SMString& s2) { return !(s1 == s2); }

}  // namespace bifrost

namespace std {

template <>
struct hash<bifrost::SMString> {
  using argument_type = bifrost::SMString;
  using result_type = std::size_t;
  result_type operator()(argument_type const& s) const noexcept { return std::hash<std::string_view>{}(s.AsView()); }
};

}  // namespace std