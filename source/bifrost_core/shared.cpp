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

#include "bifrost_core/common.h"
#include "bifrost_core/shared.h"
#include "bifrost_core/module_loader.h"
#include "bifrost_core/error.h"
#include <sstream>

#include "bifrost_shared/bifrost_shared.h"  // Only for declarations

namespace bifrost {

namespace {

static const char* TypeToString(bfs_Type type) {
  switch (type) {
    case BFS_BOOL:
      return "bool";
    case BFS_INT:
      return "int";
    case BFS_DOUBLE:
      return "double";
    case BFS_STRING:
      return "string";
    case BFS_BYTE:
      return "byte";
    default:
      __assume(0);
  }
}

static void FailedToConvert(const char* path, const char* value, uint8_t from, bfs_Type to) {
  throw std::runtime_error(
      StringFormat("Failed to read path \"%s\": Value '%s' of type %s cannot be converted to %s", path, value, TypeToString((bfs_Type)from), TypeToString(to)));
}

static std::string ConvertToString(const char* path, const bfs_Value& value) {
  switch (value.Type) {
    case BFS_BOOL:
      return std::to_string(*((bool*)&value.Value));
    case BFS_INT:
      return std::to_string(*((int*)&value.Value));
    case BFS_DOUBLE:
      return std::to_string(*((double*)&value.Value));
    case BFS_STRING:
      return std::string((const char*)value.Value, value.SizeInBytes);
    case BFS_BYTE:
      return std::string((const char*)value.Value, value.SizeInBytes);
    default:
      __assume(0);
  }
}

static bool ConvertToBool(const char* path, const bfs_Value& value) {
  switch (value.Type) {
    case BFS_BOOL:
      return (bool)value.Value;
    case BFS_INT:
      return (int)value.Value;
    case BFS_DOUBLE:
      return *((double*)&value.Value);
    case BFS_STRING:
      return (void*)value.Value != nullptr;
    case BFS_BYTE:
      return (void*)value.Value != nullptr;
    default:
      __assume(0);
  }
}

static int ConvertToInt(const char* path, const bfs_Value& value) {
  switch (value.Type) {
    case BFS_BOOL:
      return (bool)value.Value;
    case BFS_INT:
      return (int)value.Value;
    case BFS_DOUBLE:
      return static_cast<int>(*((double*)&value.Value));
    case BFS_STRING: {
      std::istringstream sout((const char*)value.Value);
      int v = 0;
      sout >> v;
      if (sout.fail()) {
        FailedToConvert(path, ConvertToString(path, value).c_str(), value.Type, BFS_INT);
      }
      return v;
    }
    case BFS_BYTE:
      FailedToConvert(path, ConvertToString(path, value).c_str(), value.Type, BFS_INT);
    default:
      __assume(0);
  }
}

static double ConvertToDouble(const char* path, const bfs_Value& value) {
  switch (value.Type) {
    case BFS_BOOL:
      return (bool)value.Value;
    case BFS_INT:
      return (int)value.Value;
    case BFS_DOUBLE:
      return *((double*)&value.Value);
    case BFS_STRING: {
      std::istringstream sout((const char*)value.Value);
      double v = 0;
      sout >> v;
      if (sout.fail()) {
        FailedToConvert(path, ConvertToString(path, value).c_str(), value.Type, BFS_DOUBLE);
      }
      return v;
    }
    case BFS_BYTE:
      FailedToConvert(path, ConvertToString(path, value).c_str(), value.Type, BFS_DOUBLE);
    default:
      __assume(0);
  }
}

static bfs_Value MakeBool(bool value) { return bfs_Value{BFS_BOOL, *((u64*)&value), sizeof(bool)}; }
static bfs_Value MakeInt(int value) { return bfs_Value{BFS_INT, *((u64*)&value), sizeof(int)}; }
static bfs_Value MakeDouble(double value) { return bfs_Value{BFS_DOUBLE, *((u64*)&value), sizeof(double)}; }
static bfs_Value MakeString(std::string_view value) { return bfs_Value{BFS_STRING, (u64)value.data(), (u32)value.size()}; }
static bfs_Value MakeByte(const void* data, u32 size) { return bfs_Value{BFS_BYTE, (u64)data, size}; }

}  // namespace

class Shared::bfs_Api {
 public:
  using bfs_Read_fn = decltype(&bfs_Read);
  bfs_Read_fn bfs_Read;

  using bfs_ReadAtomic_fn = decltype(&bfs_ReadAtomic);
  bfs_ReadAtomic_fn bfs_ReadAtomic;

  using bfs_Write_fn = decltype(&bfs_Write);
  bfs_Write_fn bfs_Write;

  using bfs_Paths_fn = decltype(&bfs_Paths);
  bfs_Paths_fn bfs_Paths;

  using bfs_FreePaths_fn = decltype(&bfs_FreePaths);
  bfs_FreePaths_fn bfs_FreePaths;

  using bfs_FreeValue_fn = decltype(&bfs_FreeValue);
  bfs_FreeValue_fn bfs_FreeValue;

  using bfs_Malloc_fn = decltype(&bfs_Malloc);
  bfs_Malloc_fn bfs_Malloc;

  using bfs_Free_fn = decltype(&bfs_Free);
  bfs_Free_fn bfs_Free;

  using bfs_StatusString_fn = decltype(&bfs_StatusString);
  bfs_StatusString_fn bfs_StatusString;

  bfs_Api() {
    auto module = ModuleLoader::Get().GetModule("bifrost_shared.dll");
    BIFROST_ASSERT_WIN_CALL((bfs_Read = (bfs_Read_fn)::GetProcAddress(module, "bfs_Read")) != NULL);
    BIFROST_ASSERT_WIN_CALL((bfs_ReadAtomic = (bfs_ReadAtomic_fn)::GetProcAddress(module, "bfs_ReadAtomic")) != NULL);
    BIFROST_ASSERT_WIN_CALL((bfs_Write = (bfs_Write_fn)::GetProcAddress(module, "bfs_Write")) != NULL);
    BIFROST_ASSERT_WIN_CALL((bfs_Paths = (bfs_Paths_fn)::GetProcAddress(module, "bfs_Paths")) != NULL);
    BIFROST_ASSERT_WIN_CALL((bfs_FreePaths = (bfs_FreePaths_fn)::GetProcAddress(module, "bfs_FreePaths")) != NULL);
    BIFROST_ASSERT_WIN_CALL((bfs_FreeValue = (bfs_FreeValue_fn)::GetProcAddress(module, "bfs_FreeValue")) != NULL);
    BIFROST_ASSERT_WIN_CALL((bfs_Malloc = (bfs_Malloc_fn)::GetProcAddress(module, "bfs_Malloc")) != NULL);
    BIFROST_ASSERT_WIN_CALL((bfs_Free = (bfs_Free_fn)::GetProcAddress(module, "bfs_Free")) != NULL);
    BIFROST_ASSERT_WIN_CALL((bfs_StatusString = (bfs_StatusString_fn)::GetProcAddress(module, "bfs_StatusString")) != NULL);
  }
};

std::unique_ptr<Shared> Shared::m_instance = nullptr;

Shared::Shared() { m_api = std::make_unique<bfs_Api>(); }

Shared::~Shared() = default;

Shared& Shared::Get() {
  if (!m_instance) {
    m_instance = std::make_unique<Shared>();
  }
  return *m_instance;
}

#define BIFROST_READ_WRITE_IMPL(TypeName, Type)                                                                         \
  Type Shared::Read##TypeName(const char* path) {                                                                       \
    bfs_Value value;                                                                                                    \
    if (bfs_Status status; (status = m_api->bfs_Read(path, &value)) != BFS_OK) {                                        \
      throw std::runtime_error(StringFormat("Failed to read path \"%s\": %s", path, m_api->bfs_StatusString(status)));  \
    }                                                                                                                   \
    return ConvertTo##TypeName(path, value);                                                                            \
  }                                                                                                                     \
  Type Shared::Read##TypeName(const char* path, Type default) {                                                         \
    bfs_Value value;                                                                                                    \
    if (bfs_Status status; (status = m_api->bfs_Read(path, &value)) != BFS_OK) {                                        \
      if (status == BFS_PATH_NOT_EXIST) return default;                                                                 \
      throw std::runtime_error(StringFormat("Failed to read path \"%s\": %s", path, m_api->bfs_StatusString(status)));  \
    }                                                                                                                   \
    return ConvertTo##TypeName(path, value);                                                                            \
  }                                                                                                                     \
  Type Shared::Read##TypeName##Atomic(const char* path) {                                                               \
    bfs_Value value;                                                                                                    \
    if (bfs_Status status; (status = m_api->bfs_ReadAtomic(path, &value)) != BFS_OK) {                                  \
      throw std::runtime_error(StringFormat("Failed to read path \"%s\": %s", path, m_api->bfs_StatusString(status)));  \
    }                                                                                                                   \
    return ConvertTo##TypeName(path, value);                                                                            \
  }                                                                                                                     \
  void Shared::Write##TypeName(const char* path, Type value) {                                                          \
    if (bfs_Status status; (status = m_api->bfs_Write(path, &Make##TypeName(value))) != BFS_OK) {                       \
      throw std::runtime_error(StringFormat("Failed to write path \"%s\": %s", path, m_api->bfs_StatusString(status))); \
    }                                                                                                                   \
  }

BIFROST_READ_WRITE_IMPL(Bool, bool)
BIFROST_READ_WRITE_IMPL(Int, int)
BIFROST_READ_WRITE_IMPL(Double, double)
BIFROST_READ_WRITE_IMPL(String, std::string)

}  // namespace bifrost