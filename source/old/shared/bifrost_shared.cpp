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

#include "bifrost/version.h"
#include "bifrost/core/macros.h"
#include "bifrost/shared/common.h"
#include "bifrost/shared/bifrost_shared.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) { return TRUE; }

BIFROST_SHARED_API bfs_Version bfs_GetVersion() { return bfs_Version{BIFROST_VERSION_MAJOR, BIFROST_VERSION_MINOR, BIFROST_VERSION_PATCH}; }

BIFROST_SHARED_API const char* bfs_GetVersionString() {
  return BIFROST_STRINGIFY(BIFROST_VERSION_MAJOR) "." BIFROST_STRINGIFY(BIFROST_VERSION_MINOR) "." BIFROST_STRINGIFY(BIFROST_VERSION_PATCH);
}

//
//#include "bifrost_shared/common.h"
//#include "bifrost_shared/bifrost_shared.h"
//#include "bifrost_shared/shared_object.h"
//#include "bifrost_shared/shared_map.h"
//#include "bifrost_shared/shared_memory.h"
//#include "bifrost_shared/shared_log_stash.h"
//#include "bifrost/core/macros.h"
//#include "bifrost/core/logging.h"
//
//using namespace bifrost;
//using namespace bifrost::shared;
//
//#define BIFROST_SHARED_CATCH_ALL(stmts) \
//  try {                                 \
//    stmts;                              \
//  } catch (std::bad_alloc&) {           \
//    return BFS_OUT_OF_MEMORY;           \
//  } catch (std::exception&) {           \
//    return BFS_UNKNOWN;                 \
//  }
//
//BIFROST_SHARED_API const char* bfs_GetVersion() {
//  return BIFROST_STRINGIFY(BIFROST_SHARED_VERSION_MAJOR) "." BIFROST_STRINGIFY(BIFROST_SHARED_VERSION_MINOR) "." BIFROST_STRINGIFY(
//      BIFROST_SHARED_VERSION_PATCH);
//}
//
//BIFROST_SHARED_API bfs_Status bfs_Read(const char* path, bfs_Value* value) {
//  BIFROST_SHARED_CATCH_ALL({ return SharedObject::Get().GetSharedMap()->Read(path, value, false); });
//}
//
//BIFROST_SHARED_API bfs_Status bfs_ReadAtomic(const char* path, bfs_Value* value) {
//  BIFROST_SHARED_CATCH_ALL({ return SharedObject::Get().GetSharedMap()->Read(path, value, true); });
//}
//
//BIFROST_SHARED_API bfs_Status bfs_Write(const char* path, const bfs_Value* value) {
//  BIFROST_SHARED_CATCH_ALL({ return SharedObject::Get().GetSharedMap()->Write(path, value); });
//}
//
//BIFROST_SHARED_API bfs_Status bfs_Paths(bfs_PathList* paths) {
//  BIFROST_SHARED_CATCH_ALL({ return SharedObject::Get().GetSharedMap()->GetPaths(paths); });
//}
//
//BIFROST_SHARED_API void bfs_FreePaths(bfs_PathList* paths) { SharedObject::Get().GetSharedMap()->FreePaths(paths); }
//
//BIFROST_SHARED_API void bfs_FreeValue(bfs_Value* value) { SharedObject::Get().GetSharedMap()->FreeValue(value); }
//
//BIFROST_SHARED_API void* bfs_Malloc(size_t value) { return SharedObject::Get().GetSharedMemory()->Allocate(value); }
//
//BIFROST_SHARED_API void bfs_Free(void* ptr) { return SharedObject::Get().GetSharedMemory()->Deallocate(ptr); }
//
//BIFROST_SHARED_API const char* bfs_StatusString(bfs_Status status) {
//  switch (status) {
//    case BFS_OK:
//      return "BFS_OK - OK";
//    case BFS_PATH_NOT_EXIST:
//      return "BFS_PATH_NOT_EXIST - Path does not exist";
//    case BFS_OUT_OF_MEMORY:
//      return "BFS_OUT_OF_MEMORY - Out of shared memory";
//    default:
//      return "BFS_UNKNOWN - Unknown error";
//  }
//}
//
//BIFROST_SHARED_API uint32_t bfs_NumBytesUnused() { return static_cast<uint32_t>(SharedObject::Get().GetSharedMemory()->GetNumFreeBytes()); }
//
//BIFROST_SHARED_API uint32_t bfs_NumBytesTotal() { return static_cast<uint32_t>(SharedObject::Get().GetSharedMemory()->GetSizeInBytes()); }
//
//BIFROST_SHARED_API bfs_Status bfs_Reset() {
//  BIFROST_SHARED_CATCH_ALL({
//    SharedObject::Unload(true);
//    SharedObject::Load(true);
//    return BFS_OK;
//  });
//}
//
//BIFROST_SHARED_API bfs_Status bfs_RegisterLogCallback(const char* name, bfs_LogCallback_t cb) {
//  BIFROST_SHARED_CATCH_ALL({ return SharedObject::Get().GetSharedLogStash()->SetCallback(name, cb); });
//}
//
//BIFROST_SHARED_API bfs_Status bfs_UnregisterLogCallback(const char* name) {
//  BIFROST_SHARED_CATCH_ALL({ return SharedObject::Get().GetSharedLogStash()->RemoveCallback(name); });
//}
//
//BIFROST_SHARED_API bfs_Status bfs_Log(int level, const char* module, const char* message) {
//  BIFROST_SHARED_CATCH_ALL({ return SharedObject::Get().GetSharedLogStash()->Push(level, module, message); });
//}
//
//BIFROST_SHARED_API bfs_Status bfs_LogStateAsync(int async) {
//  BIFROST_SHARED_CATCH_ALL({ return SharedObject::Get().GetSharedLogStash()->SetAsync(async); });
//}
//
//BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
//  switch (fdwReason) {
//    case DLL_PROCESS_ATTACH:
//      Logging::Get().SetModuleName("bifrost_shared.dll");
//      SharedObject::Load();
//      break;
//    case DLL_PROCESS_DETACH:
//      SharedObject::Unload();
//      break;
//  }
//  return TRUE;
//}