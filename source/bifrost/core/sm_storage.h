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
#include "bifrost/core/sm_string.h"
#include "bifrost/core/sm_hash_map.h"
#include "bifrost/core/sm_list.h"

namespace bifrost {

/// Shared storage value
class SMStorageValue : public SMObject {
 public:
  enum EType : u8 {
    E_Unknown = 0,
    E_Bool,
    E_Int,
    E_Double,
    E_String,
  };

  SMStorageValue();
  SMStorageValue(SMStorageValue&&) = default;
  SMStorageValue& operator=(SMStorageValue&&) = default;

  SMStorageValue(Context* ctx, bool v);
  SMStorageValue(Context* ctx, int v);
  SMStorageValue(Context* ctx, double v);
  SMStorageValue(Context* ctx, SMString v);

  /// Destruct the value
  void Destruct(SharedMemory* mem);

  /// Access the value (potential conversion)
  std::string AsString(Context* ctx) const;
  std::string_view AsStringView(Context* ctx) const;
  bool AsBool(Context* ctx) const;
  int AsInt(Context* ctx) const;
  double AsDouble(Context* ctx) const;

  /// Get the type
  EType Type() const noexcept { return m_type; }

 private:
  void FailConversion(Context* ctx, const char* to) const;

 private:
  EType m_type;
  std::array<u8, sizeof(SMString)> m_value;  // in place storage of bool, int, double or SMString
};

/// Key/value storage - unique per shared memory region (allocated in SMContext)
class SMStorage : public SMObject {
 public:
  /// Deallocate the map
  void Destruct(SharedMemory* mem);

  /// Insert a value
  void InsertBool(Context* ctx, const SMString& key, bool value);
  void InsertInt(Context* ctx, const SMString& key, int value);
  void InsertDouble(Context* ctx, const SMString& key, double value);
  void InsertString(Context* ctx, const SMString& key, std::string_view value);
  void InsertString(Context* ctx, const SMString& key, SMString value);

  /// Get a pointer to the value or return NULL
  bool GetBool(Context* ctx, const SMString& key);
  int GetInt(Context* ctx, const SMString& key);
  double GetDouble(Context* ctx, const SMString& key);
  std::string GetString(Context* ctx, const SMString& key);
  std::string_view GetStringView(Context* ctx, const SMString& key);

  /// Get the number of items in the shared storage
  u32 Size();

 private:
  SpinMutex m_mutex;
  SMHashMap<SMString, SMStorageValue> m_map;
};

}  // namespace bifrost
