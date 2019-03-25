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
#include "bifrost_shared/bifrost_shared.h"

namespace bifrost {

namespace {

template <bfs_Type Type>
inline void Convert(bool value, bfs_Value* v) {
  switch (Type) {
    case BFS_BOOL:
      v->Value = (bool)value;
      break;
    case BFS_INT:
      v->Value = (int)value;
      break;
    case BFS_DOUBLE:
      v->Value = (double)value;
      break;
    case BFS_STRING:
      // return std::to_string(value);
    case BFS_BYTE:
      // return Convert<bool, value);
    default:
      __assume(0);
  }
}

template <class T, bfs_Type Type>
inline T Convert(const bfs_Value& value) {
  switch (value.Type) {
    case BFS_BOOL: {
      bfs_Value v;
      Convert<Type>((bool)value->Value, &v);
      return (T)v->Value;
    }
    case BFS_INT:
    case BFS_DOUBLE:
      // return value;
    case BFS_STRING:
      // return std::to_string(value);
    case BFS_BYTE:
      // return Convert<bool, value);
    default:
      __assume(0);
  }
}

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

bool Shared::GetBool(const char* path) {
  bfs_Value value;
  m_api->bfs_Read(path, &value);
  return Convert<bool, BFS_BOOL>(value);
}

bool Shared::GetBool(const char* path, bool default) noexcept {}

}  // namespace bifrost