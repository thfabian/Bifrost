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

#include "bifrost_shared/common.h"
#include "bifrost_shared/shared_map.h"
#include "bifrost_shared/shared_memory.h"
#include "bifrost_shared/shared_object.h"
#include "bifrost_core/util.h"
#include "bifrost_core/mutex.h"

namespace bifrost::shared {

namespace {

static bool CanbBeStoredInPlace(const bfs_Value* value) { return value->SizeInBytes < (ArraySize(value->Padding) - 1); }

static bool HasExternalStorage(const bfs_Value* value) { return value->Type == BFS_STRING || value->Type == BFS_BYTE; }

static void* AllocNoThrow(std::size_t size) { return SharedObject::Get().GetSharedMemory()->Allocate(size); }

static void* Alloc(std::size_t size) {
  auto ptr = AllocNoThrow(size);
  if (!ptr) {
    throw std::bad_alloc();
  }
  return ptr;
}

static void Free(void* ptr) { return SharedObject::Get().GetSharedMemory()->Deallocate(ptr); }

static void Copy(const bfs_Value* from, bfs_Value* to, bool deepCopy) {
  to->Type = from->Type;
  to->SizeInBytes = from->SizeInBytes;
  if (deepCopy && HasExternalStorage(from)) {
    if (CanbBeStoredInPlace(from)) {
      to->Value = (uint64_t)to->Padding;
    } else {
      to->Value = (uint64_t)Alloc(from->SizeInBytes + 1);
    }
    std::memcpy((void*)to->Value, (void*)from->Value, from->SizeInBytes + 1);
  } else {
    if (HasExternalStorage(from) && CanbBeStoredInPlace(from)) {
      to->Value = (uint64_t)to->Padding;
      std::memcpy((void*)to->Value, (void*)from->Value, from->SizeInBytes + 1);
    } else {
      to->Value = from->Value;
    }
  }
}

}  // namespace

SharedMap::SharedMap() {}

SharedMap::~SharedMap() {
  for (const auto& pair : m_mapping) {
    FreeValue(pair.second.Data);
    Free(pair.second.Data);
  }
}

bfs_Status SharedMap::Read(std::string_view path, bfs_Value* value, bool deepCopy) {
  BIFROST_LOCK_GUARD(m_mutex);
  auto it = m_mapping.find(path);
  if (it == m_mapping.end()) return BFS_PATH_NOT_EXIST;

  Copy(it->second.Data, value, deepCopy);
  return BFS_OK;
}

bfs_Status SharedMap::Write(std::string_view path, const bfs_Value* value) {
  BIFROST_LOCK_GUARD(m_mutex);

  auto it = m_mapping.find(path);
  if (it == m_mapping.end()) {
    auto pathIt = m_paths.insert(m_paths.end(), stl::string(path.data(), path.size()));

    bfs_Value* newValue = (bfs_Value*)Alloc(sizeof(bfs_Value));
    it = m_mapping.emplace_hint(it, *pathIt, Value{newValue, pathIt});
  }

  Copy(value, it->second.Data, true);
  return BFS_OK;
}

void SharedMap::FreeValue(bfs_Value* value) {
  if (!value) return;
  if (value->Type == BFS_STRING || value->Type == BFS_STRING) {
    if (!CanbBeStoredInPlace(value)) {
      Free((void*)value->Value);
    }
  }
}

bfs_Status SharedMap::GetPaths(bfs_PathList* paths) {
  BIFROST_LOCK_GUARD(m_mutex);
  paths->Num = (uint32_t)m_mapping.size();
  paths->Paths = (char**)Alloc(sizeof(char*) * paths->Num);
  paths->Lens = (uint32_t*)Alloc(sizeof(uint32_t) * paths->Num);

  uint32_t i = 0;
  for (const auto& pair : m_mapping) {
    const std::string_view& path = pair.first;
    paths->Lens[i] = (uint32_t)path.size();
    paths->Paths[i] = (char*)Alloc(sizeof(char) * (path.size() + 1));
    std::memcpy(paths->Paths[i], path.data(), path.size());
    paths->Paths[path.size()] = '\0';
    ++i;
  }

  return BFS_OK;
}

void SharedMap::FreePaths(bfs_PathList* paths) {
  if (paths) {
    for (uint32_t i = 0; i < paths->Num; ++i) {
      Free(paths->Paths[i]);
    }
    Free((void*)paths->Lens);
    Free((void*)paths->Paths);
  }
}

}  // namespace bifrost::shared