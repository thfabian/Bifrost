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
#include "bifrost/core/module_loader.h"
#include "bifrost/core/ilogger.h"
#include "bifrost/core/sm_context.h"

namespace bifrost {

SharedMemory::SharedMemory(Context* ctx, std::string name, u64 dataSizeInBytes) : m_name(std::move(name)), m_dataSizeInBytes(dataSizeInBytes), m_ctx(ctx) {
  m_ctx->Logger().TraceFormat("Trying to allocate shared memory \"%s\" (%lu bytes) ...", GetName(), m_dataSizeInBytes);

  // Create file mapping if possible
  m_handle = ::CreateFileMappingA(INVALID_HANDLE_VALUE,  // Use paging file
                                  NULL,                  // Default security
                                  PAGE_READWRITE,        // Read/write access
                                  0, (DWORD)m_dataSizeInBytes, GetName());

  bool alreadyExist = ::GetLastError() == ERROR_ALREADY_EXISTS;
  if (alreadyExist) {
    if (m_handle != NULL) ::CloseHandle(m_handle);

    m_ctx->Logger().TraceFormat("Shared memory \"%s\" already exists, opening shared memory mapping", GetName());
    m_handle = ::OpenFileMappingA(FILE_MAP_ALL_ACCESS,  // Read/write access
                                  FALSE,                // Propagate handles
                                  GetName());
  }

  if (m_handle == NULL) {
    std::string msg = StringFormat("Failed to allocate shared memory \"%s\": %s", GetName(), GetLastWin32Error().c_str());
    m_ctx->Logger().Error(msg.c_str());
    throw std::runtime_error(msg.c_str());
  } else {
    if (alreadyExist) m_ctx->Logger().TraceFormat("Opened shared memory mapping \"%s\"", GetName());
  }

  m_startAddress = ::MapViewOfFile(m_handle,             // Handle to map object
                                   FILE_MAP_ALL_ACCESS,  // Read/write permission
                                   0, 0, m_dataSizeInBytes);

  if (m_startAddress == NULL) {
    ::CloseHandle(m_handle);
    std::string msg = StringFormat("Failed to map shared memory \"%s\": %s", GetName(), GetLastWin32Error().c_str());
    m_ctx->Logger().Error(msg.c_str());
    throw std::runtime_error(msg.c_str());
  }

#ifndef NDEBUG
  if (!alreadyExist) {
    std::memset(m_startAddress, 0xff, m_dataSizeInBytes);
  }
#endif

  // Construct the mallocator
  if (!alreadyExist) {
    m_malloc = MallocFreeList::Create(m_startAddress, m_dataSizeInBytes);
  } else {
    // We read the first 8 bytes to get the "offset" of the start address to the "this" pointer of MallocFreelist (we want the this pointer to be Cache aligned)
    m_malloc = (MallocFreeList*)((u64)m_startAddress + *((u64*)m_startAddress));
  }

  // Create the shared context
  if (!alreadyExist) {
    m_sharedCtx = SMContext::Create(this, dataSizeInBytes);
  } else {
    m_sharedCtx = SMContext::Map(GetFirstAdress());
    if (m_sharedCtx->GetMemorySize() != dataSizeInBytes) {
      m_ctx->Logger().WarnFormat("Opened shared memory's size (%lu bytes) does not match original size (%lu bytes)", dataSizeInBytes,
                                 m_sharedCtx->GetMemorySize());
    }
  }

  m_ctx->Logger().TraceFormat("Allocated shared memory \"%s\"", GetName());
}

SharedMemory::~SharedMemory() {
  SMContext::Destruct(this, m_sharedCtx);

  m_ctx->Logger().TraceFormat("Deallocating shared memory \"%s\" ...", GetName());

  if (::UnmapViewOfFile(m_startAddress) == 0) {
    m_ctx->Logger().WarnFormat("Failed to unmap shared memory \"%s\": %s", GetName(), GetLastWin32Error().c_str());
  }

  if (::CloseHandle(m_handle) == 0) {
    m_ctx->Logger().WarnFormat("Failed to deallocate shared memory: \"%s\": %s", GetName(), GetLastWin32Error().c_str());
  }

  m_ctx->Logger().TraceFormat("Deallocated shared memory \"%s\"", GetName());
}

SMLogStash* SharedMemory::GetSMLogStash() noexcept { return m_sharedCtx->GetSMLogStash(this); }

SMStorage* SharedMemory::GetSMStorage() noexcept { return m_sharedCtx->GetSMStorage(this); }

}  // namespace bifrost