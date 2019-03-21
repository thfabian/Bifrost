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

#include "bifrost_shared/bifrost_shared.h"
#include "bifrost_core/util.h"
#include "bifrost_core/mutex.h"

#define BIFROST_SHARED_UNORDERED_MAP 1
#define BIFROST_SHARED_THREAD_SAFE 1

static_assert(sizeof(bfs_Value_t) == 64, "bfs_Value_t is not cache aligned");

#if BIFROST_SHARED_THREAD_SAFE
#define BIFROST_SHARED_LOCK_GUARD(mutex) BIFROST_LOCK_GUARD(mutex)
#else
#define BIFROST_SHARED_LOCK_GUARD(mutex)
#endif

namespace {

using namespace bifrost;

class Configuration {
 public:
  struct Value {
    bfs_Value* Data;
    std::list<std::string>::iterator PathIterator;
  };

  using list_t = std::list<std::string>;
#if BIFROST_SHARED_UNORDERED_MAP
  using map_t = std::unordered_map<std::string_view, Value>;
#else
  using map_t = std::map<std::string_view, Value>;
#endif

  ~Configuration() {
    for (const auto& pair : m_mapping) {
      FreeValue(pair.second.Data);
      Free(pair.second.Data);
    }
  }

  static void OnLoad() noexcept {
    if (s_refCount++ == 0) {
      assert(s_instance == nullptr && "Configuration already allocated");
      s_instance = new Configuration;
    }
  }

  static void OnUnload() noexcept {
    if (--s_refCount == 0) {
      assert(s_instance && "Configuration not initialized");
      delete s_instance;
      s_instance = nullptr;
    }
  }

  static Configuration& Get() noexcept { return *s_instance; }

  inline int Read(std::string_view path, bfs_Value* value, bool deepCopy) noexcept {
    BIFROST_SHARED_LOCK_GUARD(m_mutex);
    auto it = m_mapping.find(path);
    if (it == m_mapping.end()) return 1;

    Copy(it->second.Data, value, deepCopy);
    return 0;
  }

  inline int Write(std::string_view path, const bfs_Value* value) noexcept {
    BIFROST_SHARED_LOCK_GUARD(m_mutex);

    auto it = m_mapping.find(path);
    if (it == m_mapping.end()) {
      auto pathIt = m_paths.insert(m_paths.end(), std::string(path.data(), path.size()));

      bfs_Value* newValue = (bfs_Value*)Alloc(sizeof(bfs_Value));
      it = m_mapping.emplace_hint(it, *pathIt, Value{newValue, pathIt});
    }

    Copy(value, it->second.Data, true);
    return 0;
  }

  inline void FreeValue(bfs_Value* value) noexcept {
    if (!value) return;
    if (value->Type == BFS_STRING || value->Type == BFS_STRING) {
      if (!CanbBeStoredInPlace(value)) {
        Free((void*)value->Value);
      }
    }
  }

  inline void GetPaths(bfs_PathList* paths) noexcept {
    BIFROST_SHARED_LOCK_GUARD(m_mutex);
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
  }

  inline void FreePaths(bfs_PathList* paths) noexcept {
    if (paths) {
      for (uint32_t i = 0; i < paths->Num; ++i) {
        Free(paths->Paths[i]);
      }
      Free((void*)paths->Lens);
      Free((void*)paths->Paths);
    }
  }

 private:
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

  static void* Alloc(std::size_t size) { return ::_aligned_malloc(size, 64); }
  static void Free(void* ptr) { return ::_aligned_free(ptr); }

  static bool CanbBeStoredInPlace(const bfs_Value* value) { return value->SizeInBytes < (ArraySize(value->Padding) - 1); }
  static bool HasExternalStorage(const bfs_Value* value) { return value->Type == BFS_STRING || value->Type == BFS_BYTE; }

 private:
  static uint32_t s_refCount;
  static Configuration* s_instance;

  std::mutex m_mutex;
  list_t m_paths;
  map_t m_mapping;
};

Configuration* Configuration::s_instance = nullptr;
uint32_t Configuration::s_refCount = 0;

}  // namespace

int bfs_Read(const char* path, bfs_Value* value) { return Configuration::Get().Read(path, value, false); }

int bfs_ReadAtomic(const char* path, bfs_Value* value) { return Configuration::Get().Read(path, value, true); }

int bfs_Write(const char* path, const bfs_Value* value) { return Configuration::Get().Write(path, value); }

void bfs_Paths(bfs_PathList* paths) { Configuration::Get().GetPaths(paths); }

void bfs_FreePaths(bfs_PathList* paths) { Configuration::Get().FreePaths(paths); }

void bfs_FreeValue(bfs_Value* value) { Configuration::Get().FreeValue(value); }

void* bfs_Malloc(size_t value) { return std::malloc(value); }

void bfs_Free(void* ptr) { std::free(ptr); }

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
  switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
      Configuration::OnLoad();
      break;
    case DLL_PROCESS_DETACH:
      Configuration::OnUnload();
      break;
  }
  return TRUE;
}