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

#include "bifrost/core/common.h"
#include "bifrost/core/sm_storage.h"
#include "bifrost/core/util.h"

namespace bifrost {

static const char* TypeToString(SMStorageValue::EType type) {
  switch (type) {
    case SMStorageValue::E_Bool:
      return "bool";
    case SMStorageValue::E_Int:
      return "int";
    case SMStorageValue::E_Double:
      return "double";
    case SMStorageValue::E_String:
      return "string";
    case SMStorageValue::E_Unknown:
    default:
      return "unknown";
  }
}

SMStorageValue::SMStorageValue() : m_type(E_Unknown), m_value() {}

SMStorageValue::SMStorageValue(Context* ctx, bool v) : m_type(E_Bool) { m_value.Bool = v; }

SMStorageValue::SMStorageValue(Context* ctx, int v) : m_type(E_Int) { m_value.Int = v; }

SMStorageValue::SMStorageValue(Context* ctx, double v) : m_type(E_Double) { m_value.Double = v;}

SMStorageValue::SMStorageValue(Context* ctx, SMString v) : m_type(E_String)  { m_value.String = std::move(v); }

void SMStorageValue::Move(SMStorageValue&& s) {
  m_type = s.m_type;
  switch (m_type) {
    case SMStorageValue::E_Bool:
      m_value.Bool = s.m_value.Bool;
      break;
    case SMStorageValue::E_Int:
      m_value.Int = s.m_value.Int;
      break;
    case SMStorageValue::E_Double:
      m_value.Double = s.m_value.Double;
      break;
    case SMStorageValue::E_String:
      m_value.String = std::move(s.m_value.String);
      break;
    case SMStorageValue::E_Unknown:
    default:
      break;
  }
}

SMStorageValue::SMStorageValue(SMStorageValue&& s) { Move(std::forward<SMStorageValue>(s)); }

SMStorageValue& SMStorageValue::operator=(SMStorageValue&& s) {
  Move(std::forward<SMStorageValue>(s));
  return *this;
}

 void SMStorageValue::Destruct(SharedMemory* mem) {
  if (m_type == E_String) {
    m_value.String.Destruct(mem);
  }
}

std::string_view SMStorageValue::AsStringView(Context* ctx) const {
  if (m_type == E_String) {
    return m_value.String.AsView(ctx);
  } else {
    FailConversion(ctx, "string view");
  }
}

std::string SMStorageValue::AsString(Context* ctx) const {
  switch (m_type) {
    case SMStorageValue::E_Bool:
      return std::to_string(m_value.Bool);
    case SMStorageValue::E_Int:
      return std::to_string(m_value.Int);
    case SMStorageValue::E_Double:
      return std::to_string(m_value.Double);
    case SMStorageValue::E_String:
      return m_value.String.AsString(ctx);
    case SMStorageValue::E_Unknown:
    default:
      FailConversion(ctx, "string");
  }
}

bool SMStorageValue::AsBool(Context* ctx) const {
  switch (m_type) {
    case SMStorageValue::E_Bool:
      return m_value.Bool;
    case SMStorageValue::E_Int:
      return m_value.Int;
    case SMStorageValue::E_Double:
      return m_value.Double;
    case SMStorageValue::E_String: {
      std::istringstream sout(m_value.String.AsString(ctx));
      bool v = 0;
      sout >> v;
      if (sout.fail()) FailConversion(ctx, "bool");
      return v;
    }
    case SMStorageValue::E_Unknown:
    default:
      FailConversion(ctx, "bool");
  }
}

int SMStorageValue::AsInt(Context* ctx) const {
  switch (m_type) {
    case SMStorageValue::E_Bool:
      return m_value.Bool;
    case SMStorageValue::E_Int:
      return m_value.Int;
    case SMStorageValue::E_Double:
      return static_cast<int>(m_value.Double);
    case SMStorageValue::E_String: {
      std::istringstream sout(m_value.String.AsString(ctx));
      int v = 0;
      sout >> v;
      if (sout.fail()) FailConversion(ctx, "int");
      return v;
    }
    case SMStorageValue::E_Unknown:
    default:
      FailConversion(ctx, "int");
  }
}

double SMStorageValue::AsDouble(Context* ctx) const {
  switch (m_type) {
    case SMStorageValue::E_Bool:
      return m_value.Bool;
    case SMStorageValue::E_Int:
      return m_value.Int;
    case SMStorageValue::E_Double:
      return m_value.Double;
    case SMStorageValue::E_String: {
      std::istringstream sout(m_value.String.AsString(ctx));
      double v = 0;
      sout >> v;
      if (sout.fail()) FailConversion(ctx, "double");
      return v;
    }
    case SMStorageValue::E_Unknown:
    default:
      FailConversion(ctx, "double");
  }
}

[[noreturn]] void SMStorageValue::FailConversion(Context* ctx, const char* to) const {
  throw std::domain_error(StringFormat("cannot convert value \"%s\" of type '%s' to '%s'", AsString(ctx).c_str(), TypeToString(m_type), to));
}

void SMStorage::Destruct(SharedMemory* mem) { m_map.Destruct(mem); }

void SMStorage::InsertBool(Context* ctx, std::string_view key, bool value) {
  BIFROST_LOCK_GUARD(m_mutex);
  m_keyBuffer.Assign(ctx, key);
  m_map.Insert(ctx, m_keyBuffer, {ctx, value});
}

void SMStorage::InsertInt(Context* ctx, std::string_view key, int value) {
  BIFROST_LOCK_GUARD(m_mutex);
  m_keyBuffer.Assign(ctx, key);
  m_map.Insert(ctx, m_keyBuffer, {ctx, value});
}

void SMStorage::InsertDouble(Context* ctx, std::string_view key, double value) {
  BIFROST_LOCK_GUARD(m_mutex);
  m_keyBuffer.Assign(ctx, key);
  m_map.Insert(ctx, m_keyBuffer, {ctx, value});
}

void SMStorage::InsertString(Context* ctx, std::string_view key, std::string_view value) { InsertString(ctx, key, {ctx, value}); }

void SMStorage::InsertString(Context* ctx, std::string_view key, SMString value) {
  BIFROST_LOCK_GUARD(m_mutex);
  m_keyBuffer.Assign(ctx, key);
  m_map.Insert(ctx, m_keyBuffer, {ctx, std::move(value)});
}

bool SMStorage::GetBool(Context* ctx, std::string_view key) {
  const SMStorageValue* value = nullptr;
  {
    BIFROST_LOCK_GUARD(m_mutex);
    m_keyBuffer.Assign(ctx, key);
    value = m_map.Get(ctx, m_keyBuffer);
  }

  if (!value) {
    throw std::runtime_error(StringFormat("Key \"%s\" does not exist", key.data()).c_str());
  }

  try {
    return value->AsBool(ctx);
  } catch (std::domain_error& e) {
    throw std::runtime_error(StringFormat("Failed to convert value of key \"%s\": %s", key.data(), e.what()).c_str());
  }
  __assume(0);
}

int SMStorage::GetInt(Context* ctx, std::string_view key) {
  const SMStorageValue* value = nullptr;
  {
    BIFROST_LOCK_GUARD(m_mutex);
    m_keyBuffer.Assign(ctx, key);
    value = m_map.Get(ctx, m_keyBuffer);
  }

  if (!value) {
    throw std::runtime_error(StringFormat("Key \"%s\" does not exist", key.data()).c_str());
  }

  try {
    return value->AsInt(ctx);
  } catch (std::domain_error& e) {
    throw std::runtime_error(StringFormat("Failed to convert value of key \"%s\": %s", key.data(), e.what()).c_str());
  }
  __assume(0);
}

double SMStorage::GetDouble(Context* ctx, std::string_view key) {
  const SMStorageValue* value = nullptr;
  {
    BIFROST_LOCK_GUARD(m_mutex);
    m_keyBuffer.Assign(ctx, key);
    value = m_map.Get(ctx, m_keyBuffer);
  }

  if (!value) {
    throw std::runtime_error(StringFormat("Key \"%s\" does not exist", key.data()).c_str());
  }

  try {
    return value->AsDouble(ctx);
  } catch (std::domain_error& e) {
    throw std::runtime_error(StringFormat("Failed to convert value of key \"%s\": %s", key.data(), e.what()).c_str());
  }
  __assume(0);
}

std::string SMStorage::GetString(Context* ctx, std::string_view key) {
  const SMStorageValue* value = nullptr;
  {
    BIFROST_LOCK_GUARD(m_mutex);
    m_keyBuffer.Assign(ctx, key);
    value = m_map.Get(ctx, m_keyBuffer);
  }

  if (!value) {
    throw std::runtime_error(StringFormat("Key \"%s\" does not exist", key.data()).c_str());
  }

  try {
    return value->AsString(ctx);
  } catch (std::domain_error& e) {
    throw std::runtime_error(StringFormat("Failed to convert value of key \"%s\": %s", key.data(), e.what()).c_str());
  }
  __assume(0);
}

std::string_view SMStorage::GetStringView(Context* ctx, std::string_view key) {
  const SMStorageValue* value = nullptr;
  {
    BIFROST_LOCK_GUARD(m_mutex);
    m_keyBuffer.Assign(ctx, key);
    value = m_map.Get(ctx, m_keyBuffer);
  }

  if (!value) {
    throw std::runtime_error(StringFormat("Key \"%s\" does not exist", key.data()).c_str());
  }

  try {
    return value->AsStringView(ctx);
  } catch (std::domain_error& e) {
    throw std::runtime_error(StringFormat("Failed to convert value of key \"%s\": %s", key.data(), e.what()).c_str());
  }
  __assume(0);
}

bool SMStorage::Remove(Context* ctx, std::string_view key) {
  BIFROST_LOCK_GUARD(m_mutex);
  m_keyBuffer.Assign(ctx, key);
  return m_map.Remove(ctx, m_keyBuffer);
}

bifrost::u32 SMStorage::Size() {
  BIFROST_LOCK_GUARD(m_mutex);
  return m_map.Size();
}

void SMStorage::Clear(Context* ctx) {
  m_map.Clear(ctx);
  m_keyBuffer.Clear(ctx);
}

}  // namespace bifrost