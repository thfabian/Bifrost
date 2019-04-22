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
  SMStorageValue(SMStorageValue&&);
  SMStorageValue& operator=(SMStorageValue&&);

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
  void Move(SMStorageValue&& s);

 private:
  EType m_type;

  union Value {
    bool Bool;
    int Int;
    double Double;
    SMString String;

    Value() : String() {}
  } m_value;
};

/// Key/value storage - unique per shared memory region (allocated in SMContext)
class SMStorage : public SMObject {
 public:
  /// Deallocate the map
  void Destruct(SharedMemory* mem);

  /// Insert a value
  void InsertBool(Context* ctx, std::string_view key, bool value);
  void InsertInt(Context* ctx, std::string_view key, int value);
  void InsertDouble(Context* ctx, std::string_view key, double value);
  void InsertString(Context* ctx, std::string_view key, std::string_view value);
  void InsertString(Context* ctx, std::string_view key, SMString value);

  /// Get the value or throw
  bool GetBool(Context* ctx, std::string_view key);
  int GetInt(Context* ctx, std::string_view key);
  double GetDouble(Context* ctx, std::string_view key);
  std::string GetString(Context* ctx, std::string_view key);
  std::string_view GetStringView(Context* ctx, std::string_view key);

  /// Remove the given key
  bool Remove(Context* ctx, std::string_view key);

  /// Get the number of items in the shared storage
  u32 Size();

  /// Clear the storage
  void Clear(Context* ctx);

 private:
  SpinMutex m_mutex;
  SMString m_keyBuffer;
  SMHashMap<SMString, SMStorageValue> m_map;
};

}  // namespace bifrost
