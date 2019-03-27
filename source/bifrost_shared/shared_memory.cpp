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
#include "bifrost_shared/shared_memory.h"
#include "bifrost_core/logging.h"
#include "bifrost_core/error.h"

namespace bifrost::shared {

SharedMemory::~SharedMemory() {
  BIFROST_LOG_DEBUG("Deallocating shared memory \"%s\" ...", GetName());

  if (::UnmapViewOfFile(m_startAddress) == 0) {
    BIFROST_LOG_WARN("Failed to unmap shared memory \"%s\": %s", GetName(), GetLastWin32Error().c_str());
  }

  if (::CloseHandle(m_handle) == 0) {
    BIFROST_LOG_WARN("Failed to deallocate shared memory: \"%s\": %s", GetName(), GetLastWin32Error().c_str());
  }

  BIFROST_LOG_DEBUG("Successfully deallocated shared memory \"%s\"", GetName());
}

SharedMemory::SharedMemory(const Config& config) : m_config(config) {
  BIFROST_LOG_DEBUG("Allocating shared memory \"%s\" (size = %lu) ...", m_config.Name.c_str(), m_config.DataSizeInBytes);

  // Check if the file already exists
  m_handle = ::CreateFileMappingA(INVALID_HANDLE_VALUE,  // use paging file
                                  NULL,                  // default security
                                  PAGE_READWRITE,        // read/write access
                                  0, (DWORD) m_config.DataSizeInBytes, GetName());
  bool sharedMemCreated = m_handle != NULL;

  if (m_handle == NULL || ::GetLastError() == ERROR_ALREADY_EXISTS) {
    m_handle = ::OpenFileMappingA(FILE_MAP_ALL_ACCESS,  // read/write access
                                  FALSE,                // do not inherit the name
                                  GetName());
  }

  if (m_handle == NULL) {
    throw std::runtime_error(StringFormat("Failed to allocate shared memory \"%s\": %s", GetName(), GetLastWin32Error().c_str()));
  }

  m_startAddress = ::MapViewOfFile(m_handle,             // handle to map object
                                   FILE_MAP_ALL_ACCESS,  // read/write permission
                                   0, 0, m_config.DataSizeInBytes);

  if (m_startAddress == NULL) {
    ::CloseHandle(m_handle);
    throw std::runtime_error(StringFormat("Failed to map shared memory \"%s\": %s", GetName(), GetLastWin32Error().c_str()));
  }

#ifdef _DEBUG
  if (sharedMemCreated) {
    std::memset(m_start_address, 0xff, m_config.DataSizeInBytes);
  }
#endif

  // Construct the mallocator
  if (sharedMemCreated) {
    m_malloc = MallocFreeList::Create(m_startAddress, m_config.DataSizeInBytes);
  } else {
    m_malloc = (MallocFreeList*)((u64)m_startAddress + *((u64*)m_startAddress));
  }

  BIFROST_LOG_DEBUG("Successfully allocated shared memory \"%s\"", GetName());
}

}  // namespace bifrost::shared