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
#include "bifrost/core/error.h"
#include "bifrost/core/shared_memory.h"
#include "bifrost/core/ilogger.h"

namespace bifrost {

SharedMemory::SharedMemory(Context* ctx, std::string name, u64 dataSizeInBytes) : m_name(std::move(name)), m_dataSizeInBytes(dataSizeInBytes), m_ctx(ctx) {
  m_ctx->Logger().DebugFormat("Trying to allocating shared memory \"%s\" (size = %lu) ...", GetName(), m_dataSizeInBytes);

  // Check if the file already exists
  m_handle = ::CreateFileMappingA(INVALID_HANDLE_VALUE,  // use paging file
                                  NULL,                  // default security
                                  PAGE_READWRITE,        // read/write access
                                  0, (DWORD)m_dataSizeInBytes, GetName());
  bool sharedMemCreated = m_handle != NULL;

  if (m_handle == NULL || ::GetLastError() == ERROR_ALREADY_EXISTS) {
    m_ctx->Logger().DebugFormat("Shared memory \"%s\" already exists, opening shared memory mapping", GetName());
    m_handle = ::OpenFileMappingA(FILE_MAP_ALL_ACCESS,  // read/write access
                                  FALSE,                // do not inherit the name
                                  GetName());
  }

  if (m_handle == NULL) {
    std::string msg = StringFormat("Failed to allocate shared memory \"%s\": %s", GetName(), GetLastWin32Error().c_str());
    m_ctx->Logger().Error(msg.c_str());
    throw std::runtime_error(msg.c_str());
  } else {
    if (!sharedMemCreated) m_ctx->Logger().DebugFormat("Opened shared memory mapping \"%s\"", GetName());
  }

  m_startAddress = ::MapViewOfFile(m_handle,             // handle to map object
                                   FILE_MAP_ALL_ACCESS,  // read/write permission
                                   0, 0, m_dataSizeInBytes);

  if (m_startAddress == NULL) {
    ::CloseHandle(m_handle);
    std::string msg = StringFormat("Failed to map shared memory \"%s\": %s", GetName(), GetLastWin32Error().c_str());
    m_ctx->Logger().Error(msg.c_str());
    throw std::runtime_error(msg.c_str());
  }

#ifndef NDEBUG
  if (sharedMemCreated) {
    std::memset(m_startAddress, 0xff, m_dataSizeInBytes);
  }
#endif

  // Construct the mallocator
  if (sharedMemCreated) {
    m_malloc = MallocFreeList::Create(m_startAddress, m_dataSizeInBytes);
  } else {
    // We read the first 8 bytes to get the "offset" of the start address to the "this" pointer of MallocFreelist (we want the this pointer to be Cache aligned)
    m_malloc = (MallocFreeList*)((u64)m_startAddress + *((u64*)m_startAddress));
  }

  m_ctx->Logger().DebugFormat("Allocated shared memory \"%s\"", GetName());
}

SharedMemory::~SharedMemory() {
  m_ctx->Logger().DebugFormat("Deallocating shared memory \"%s\" ...", GetName());

  if (::UnmapViewOfFile(m_startAddress) == 0) {
    m_ctx->Logger().WarnFormat("Failed to unmap shared memory \"%s\": %s", GetName(), GetLastWin32Error().c_str());
  }

  if (::CloseHandle(m_handle) == 0) {
    m_ctx->Logger().WarnFormat("Failed to deallocate shared memory: \"%s\": %s", GetName(), GetLastWin32Error().c_str());
  }

  m_ctx->Logger().DebugFormat("Deallocated shared memory \"%s\"", GetName());
}

}  // namespace bifrost