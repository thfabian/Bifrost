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

SMStorageValue::SMStorageValue() : m_type(E_Unknown) {}

SMStorageValue::SMStorageValue(Context* ctx, bool v) : m_type(E_Bool) { std::memcpy(m_value.data(), &v, sizeof(v)); }

SMStorageValue::SMStorageValue(Context* ctx, int v) : m_type(E_Int) { std::memcpy(m_value.data(), &v, sizeof(v)); }

SMStorageValue::SMStorageValue(Context* ctx, double v) : m_type(E_Double) { std::memcpy(m_value.data(), &v, sizeof(v)); }

SMStorageValue::SMStorageValue(Context* ctx, SMString v) : m_type(E_String) { std::memcpy(m_value.data(), &v, sizeof(v)); }

void SMStorageValue::Destruct(SharedMemory* mem) {
  if (m_type == E_String) {
    ((SMString*)m_value.data())->Destruct(mem);
  }
}

std::string_view SMStorageValue::AsStringView(Context* ctx) const {
  if (m_type == E_String) {
    return ((SMString*)m_value.data())->AsView(ctx);
  } else {
    FailConversion(ctx, "string view");
  }
}

std::string SMStorageValue::AsString(Context* ctx) const {
  switch (m_type) {
    case SMStorageValue::E_Bool:
      return std::to_string(*(bool*)m_value.data());
    case SMStorageValue::E_Int:
      return std::to_string(*(int*)m_value.data());
    case SMStorageValue::E_Double:
      return std::to_string(*(double*)m_value.data());
    case SMStorageValue::E_String:
      return ((SMString*)m_value.data())->AsString(ctx);
    case SMStorageValue::E_Unknown:
    default:
      FailConversion(ctx, "string");
  }
}

bool SMStorageValue::AsBool(Context* ctx) const {
  switch (m_type) {
    case SMStorageValue::E_Bool:
      return *(bool*)m_value.data();
    case SMStorageValue::E_Int:
      return *(int*)m_value.data();
    case SMStorageValue::E_Double:
      return *(double*)m_value.data();
    case SMStorageValue::E_String: {
      std::istringstream sout(((SMString*)m_value.data())->AsString(ctx));
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
      return *(bool*)m_value.data();
    case SMStorageValue::E_Int:
      return *(int*)m_value.data();
    case SMStorageValue::E_Double:
      return static_cast<int>(*(double*)m_value.data());
    case SMStorageValue::E_String: {
      std::istringstream sout(((SMString*)m_value.data())->AsString(ctx));
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
      return *(bool*)m_value.data();
    case SMStorageValue::E_Int:
      return *(int*)m_value.data();
    case SMStorageValue::E_Double:
      return *(double*)m_value.data();
    case SMStorageValue::E_String: {
      std::istringstream sout(((SMString*)m_value.data())->AsString(ctx));
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

void SMStorage::InsertBool(Context* ctx, const SMString& key, bool value) {
  BIFROST_LOCK_GUARD(m_mutex);
  m_map.Insert(ctx, key, {ctx, value});
}

void SMStorage::InsertInt(Context* ctx, const SMString& key, int value) {
  BIFROST_LOCK_GUARD(m_mutex);
  m_map.Insert(ctx, key, {ctx, value});
}

void SMStorage::InsertDouble(Context* ctx, const SMString& key, double value) {
  BIFROST_LOCK_GUARD(m_mutex);
  m_map.Insert(ctx, key, {ctx, value});
}

void SMStorage::InsertString(Context* ctx, const SMString& key, std::string_view value) { InsertString(ctx, key, {ctx, value}); }

void SMStorage::InsertString(Context* ctx, const SMString& key, SMString value) {
  BIFROST_LOCK_GUARD(m_mutex);
  m_map.Insert(ctx, key, {ctx, std::move(value)});
}

bool SMStorage::GetBool(Context* ctx, const SMString& key) {
  const SMStorageValue* value = nullptr;
  {
    BIFROST_LOCK_GUARD(m_mutex);
    value = m_map.Get(ctx, key);
  }

  if (!value) {
    throw new std::runtime_error(StringFormat("Key \"%s\" does not exist", key.AsView(ctx).data()).c_str());
  }

  try {
    return value->AsBool(ctx);
  } catch (std::domain_error& e) {
    throw new std::runtime_error(StringFormat("Failed to convert value of key \"%s\": %s", key.AsView(ctx).data(), e.what()).c_str());
  }
  __assume(0);
}

int SMStorage::GetInt(Context* ctx, const SMString& key) {
  const SMStorageValue* value = nullptr;
  {
    BIFROST_LOCK_GUARD(m_mutex);
    value = m_map.Get(ctx, key);
  }

  if (!value) {
    throw new std::runtime_error(StringFormat("Key \"%s\" does not exist", key.AsView(ctx).data()).c_str());
  }

  try {
    return value->AsInt(ctx);
  } catch (std::domain_error& e) {
    throw new std::runtime_error(StringFormat("Failed to convert value of key \"%s\": %s", key.AsView(ctx).data(), e.what()).c_str());
  }
  __assume(0);
}

double SMStorage::GetDouble(Context* ctx, const SMString& key) {
  const SMStorageValue* value = nullptr;
  {
    BIFROST_LOCK_GUARD(m_mutex);
    value = m_map.Get(ctx, key);
  }

  if (!value) {
    throw new std::runtime_error(StringFormat("Key \"%s\" does not exist", key.AsView(ctx).data()).c_str());
  }

  try {
    return value->AsDouble(ctx);
  } catch (std::domain_error& e) {
    throw new std::runtime_error(StringFormat("Failed to convert value of key \"%s\": %s", key.AsView(ctx).data(), e.what()).c_str());
  }
  __assume(0);
}

std::string SMStorage::GetString(Context* ctx, const SMString& key) {
  const SMStorageValue* value = nullptr;
  {
    BIFROST_LOCK_GUARD(m_mutex);
    value = m_map.Get(ctx, key);
  }

  if (!value) {
    throw new std::runtime_error(StringFormat("Key \"%s\" does not exist", key.AsView(ctx).data()).c_str());
  }

  try {
    return value->AsString(ctx);
  } catch (std::domain_error& e) {
    throw new std::runtime_error(StringFormat("Failed to convert value of key \"%s\": %s", key.AsView(ctx).data(), e.what()).c_str());
  }
  __assume(0);
}

std::string_view SMStorage::GetStringView(Context* ctx, const SMString& key) {
  const SMStorageValue* value = nullptr;
  {
    BIFROST_LOCK_GUARD(m_mutex);
    value = m_map.Get(ctx, key);
  }

  if (!value) {
    throw new std::runtime_error(StringFormat("Key \"%s\" does not exist", key.AsView(ctx).data()).c_str());
  }

  try {
    return value->AsStringView(ctx);
  } catch (std::domain_error& e) {
    throw new std::runtime_error(StringFormat("Failed to convert value of key \"%s\": %s", key.AsView(ctx).data(), e.what()).c_str());
  }
  __assume(0);
}

bifrost::u32 SMStorage::Size() {
  BIFROST_LOCK_GUARD(m_mutex);
  return m_map.Size();
}

}  // namespace bifrost