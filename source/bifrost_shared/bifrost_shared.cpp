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

#include "bifrost_shared/common.h"
#include "bifrost_shared/bifrost_shared.h"
#include "bifrost_shared/shared_memory.h"
#include "bifrost_core/util.h"
#include "bifrost_core/mutex.h"

#define BIFROST_SHARED_MEMORY_NAME "__bifrost_shared__"
#define BIFROST_SHARED_MEMORY_SIZE 1 << 20  // 1 MB

namespace {

using namespace bifrost;
using namespace bifrost::shared;

class Configuration {
 public:
  template <class T>
  using allocator_t = SharedMemoryAllocator<T>;

  using string_allocator_t = allocator_t<char>;
  using string_t = std::basic_string<char, std::char_traits<char>, string_allocator_t>;

  using list_allocator_t = allocator_t<string_t>;
  using list_t = std::list<string_t, list_allocator_t>;

  struct Value {
    bfs_Value* Data;
    list_t::iterator PathIterator;
  };

  using map_allocator_t = allocator_t<std::pair<const std::string_view, Value>>;
  using map_t = std::unordered_map<std::string_view, Value, std::hash<std::string_view>, std::equal_to<std::string_view>, map_allocator_t>;

  Configuration() : m_mapping(map_allocator_t(*s_mem)), m_paths(list_allocator_t(*s_mem)) {}

  ~Configuration() {
    for (const auto& pair : m_mapping) {
      FreeValue(pair.second.Data);
      Free(pair.second.Data);
    }
  }

  static void OnLoad() noexcept {
    if (s_refCount++ == 0) {
      Init(BIFROST_SHARED_MEMORY_SIZE);
    }
  }

  static void Init(u64 smemSize) noexcept {
    assert(s_mem == nullptr && "Shared memory already allocated");
    s_mem = new SharedMemory(SharedMemory::Config{BIFROST_SHARED_MEMORY_NAME, smemSize});

    assert(s_instance == nullptr && "Configuration already allocated");
    s_instance = (Configuration*)Configuration::Alloc(sizeof(Configuration));
    ::new (s_instance) Configuration();
  }

  static void Finalize() noexcept {
    if (s_instance) {
      s_instance->~Configuration();
      s_mem->Deallocate(s_instance);
      s_instance = nullptr;
    }
    if (s_mem) {
      delete s_mem;
      s_mem = nullptr;
    }
  }

  static void OnUnload() noexcept {
    if (--s_refCount == 0) {
      Finalize();
    }
  }

  static Configuration& Get() noexcept { return *s_instance; }

  inline bfs_Status Read(std::string_view path, bfs_Value* value, bool deepCopy) noexcept {
    BIFROST_LOCK_GUARD(m_mutex);
    auto it = m_mapping.find(path);
    if (it == m_mapping.end()) return BFS_PATH_NOT_EXIST;

    Copy(it->second.Data, value, deepCopy);
    return BFS_OK;
  }

  inline bfs_Status Write(std::string_view path, const bfs_Value* value) noexcept {
    BIFROST_LOCK_GUARD(m_mutex);

    auto it = m_mapping.find(path);
    if (it == m_mapping.end()) {
      auto pathIt = m_paths.insert(m_paths.end(), string_t(path.data(), path.size(), string_allocator_t(*s_mem)));

      bfs_Value* newValue = (bfs_Value*)Alloc(sizeof(bfs_Value));
      it = m_mapping.emplace_hint(it, *pathIt, Value{newValue, pathIt});
    }

    Copy(value, it->second.Data, true);
    return BFS_OK;
  }

  inline void FreeValue(bfs_Value* value) noexcept {
    if (!value) return;
    if (value->Type == BFS_STRING || value->Type == BFS_STRING) {
      if (!CanbBeStoredInPlace(value)) {
        Free((void*)value->Value);
      }
    }
  }

  inline bfs_Status GetPaths(bfs_PathList* paths) noexcept {
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

  inline void FreePaths(bfs_PathList* paths) noexcept {
    if (paths) {
      for (uint32_t i = 0; i < paths->Num; ++i) {
        Free(paths->Paths[i]);
      }
      Free((void*)paths->Lens);
      Free((void*)paths->Paths);
    }
  }

  static void* Alloc(std::size_t size) {
    auto ptr = AllocNoThrow(size);
    if (!ptr) {
      throw std::bad_alloc();
    }
    return ptr;
  }
  static void* AllocNoThrow(std::size_t size) { return s_mem->Allocate(size); }
  static void Free(void* ptr) { return s_mem->Deallocate(ptr); }

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

  static bool CanbBeStoredInPlace(const bfs_Value* value) { return value->SizeInBytes < (ArraySize(value->Padding) - 1); }
  static bool HasExternalStorage(const bfs_Value* value) { return value->Type == BFS_STRING || value->Type == BFS_BYTE; }

 private:
  static uint32_t s_refCount;
  static SharedMemory* s_mem;
  static Configuration* s_instance;

  std::mutex m_mutex;
  list_t m_paths;
  map_t m_mapping;
};

Configuration* Configuration::s_instance = nullptr;
SharedMemory* Configuration::s_mem = nullptr;
uint32_t Configuration::s_refCount = 0;

}  // namespace

#define BIFROST_SHARED_CATCH_ALL(stmt) \
  try {                                \
    return stmt;                       \
  } catch (std::bad_alloc&) {          \
    return BFS_OUT_OF_MEMORY;          \
  } catch (std::exception&) {          \
    return BFS_UNKNOWN;                \
  }

bfs_Status bfs_Read(const char* path, bfs_Value* value) { BIFROST_SHARED_CATCH_ALL(Configuration::Get().Read(path, value, false)); }

bfs_Status bfs_ReadAtomic(const char* path, bfs_Value* value) { BIFROST_SHARED_CATCH_ALL(Configuration::Get().Read(path, value, true)); }

bfs_Status bfs_Write(const char* path, const bfs_Value* value) { BIFROST_SHARED_CATCH_ALL(Configuration::Get().Write(path, value)); }

bfs_Status bfs_Paths(bfs_PathList* paths) { BIFROST_SHARED_CATCH_ALL(Configuration::Get().GetPaths(paths)); }

void bfs_FreePaths(bfs_PathList* paths) { Configuration::Get().FreePaths(paths); }

void bfs_FreeValue(bfs_Value* value) { Configuration::Get().FreeValue(value); }

void* bfs_Malloc(size_t value) { return Configuration::AllocNoThrow(value); }

void bfs_Free(void* ptr) { return Configuration::Free(ptr); }

BIFROST_SHARED_API const char* bfs_StatusString(bfs_Status status) {
  switch (status) {
    case BFS_OK:
      return "BFS_OK - OK";
    case BFS_PATH_NOT_EXIST:
      return "BFS_PATH_NOT_EXIST - Path does not exist";
    case BFS_OUT_OF_MEMORY:
      return "BFS_OUT_OF_MEMORY - Out of shared memory";
    default:
      return "BFS_UNKNOWN - Unknown error";
  }
}

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