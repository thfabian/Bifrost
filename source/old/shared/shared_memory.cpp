////   ____  _  __               _
////  |  _ \(_)/ _|             | |
////  | |_) |_| |_ _ __ ___  ___| |_
////  |  _ <| |  _| '__/ _ \/ __| __|
////  | |_) | | | | | | (_) \__ \ |_
////  |____/|_|_| |_|  \___/|___/\__|   2018 - 2019
////
////
//// This file is distributed under the MIT License (MIT).
//// See LICENSE.txt for details.
//
//#include "bifrost_shared/common.h"
//#include "bifrost_shared/shared_memory.h"
//#include "bifrost/core/logging.h"
//#include "bifrost/core/error.h"
//#include "bifrost/core/module_loader.h"
//#include <sstream>
//
//namespace bifrost::shared {
//
//SharedMemory::SharedMemory(const Config& config) : m_config(config), m_file(nullptr) {
//#ifdef NDEBUG
//  bool debug = false;
//#else
//  bool debug = true;
//#endif
//
//  if (debug || std::getenv("BIFROST_SHARED_MEMORY_DEBUG") || std::getenv("BIFROST_DEBUG")) {
//    m_file = std::fopen("shared_memory.log.txt", "a");
//    m_module = ModuleLoader::Get().GetCurrentModuleName();
//  }
//
//  LogDebug(StringFormat("Trying to allocating shared memory \"%s\" (size = %lu)", m_config.Name.c_str(), m_config.DataSizeInBytes));
//
//  // Check if the file already exists
//  m_handle = ::CreateFileMappingA(INVALID_HANDLE_VALUE,  // use paging file
//                                  NULL,                  // default security
//                                  PAGE_READWRITE,        // read/write access
//                                  0, (DWORD)m_config.DataSizeInBytes, GetName());
//  bool sharedMemCreated = m_handle != NULL;
//
//  if (m_handle == NULL || ::GetLastError() == ERROR_ALREADY_EXISTS) {
//    LogDebug(StringFormat("Shared memory \"%s\" already exists, opening shared memory mapping", m_config.Name.c_str()));
//    m_handle = ::OpenFileMappingA(FILE_MAP_ALL_ACCESS,  // read/write access
//                                  FALSE,                // do not inherit the name
//                                  GetName());
//  }
//
//  if (m_handle == NULL) {
//    auto msg = StringFormat("Failed to allocate shared memory \"%s\": %s", GetName(), GetLastWin32Error().c_str());
//    LogError(msg);
//    if (m_file) {
//      std::fclose(m_file);
//    }
//    throw std::runtime_error(msg.c_str());
//  } else {
//    if (!sharedMemCreated) LogDebug(StringFormat("Opened shared memory mapping \"%s\"", m_config.Name.c_str()));
//  }
//
//  m_startAddress = ::MapViewOfFile(m_handle,             // handle to map object
//                                   FILE_MAP_ALL_ACCESS,  // read/write permission
//                                   0, 0, m_config.DataSizeInBytes);
//
//  if (m_startAddress == NULL) {
//    ::CloseHandle(m_handle);
//    auto msg = StringFormat("Failed to map shared memory \"%s\": %s", GetName(), GetLastWin32Error().c_str());
//    LogError(msg);
//    if (m_file) {
//      std::fclose(m_file);
//    }
//    throw std::runtime_error(msg.c_str());
//  }
//
//#ifndef NDEBUG
//  if (sharedMemCreated) {
//    std::memset(m_startAddress, 0xff, m_config.DataSizeInBytes);
//  }
//#endif
//
//  // Construct the mallocator
//  if (sharedMemCreated) {
//    m_malloc = MallocFreeList::Create(m_startAddress, m_config.DataSizeInBytes);
//  } else {
//    m_malloc = (MallocFreeList*)((u64)m_startAddress + *((u64*)m_startAddress));
//  }
//
//  LogDebug(StringFormat("Allocated shared memory \"%s\"", GetName()));
//}
//
//SharedMemory::~SharedMemory() {
//  LogDebug(StringFormat("Deallocating shared memory \"%s\"", GetName()));
//
//  if (::UnmapViewOfFile(m_startAddress) == 0) {
//    LogWarn(StringFormat("Failed to unmap shared memory \"%s\": %s", GetName(), GetLastWin32Error().c_str()));
//  }
//
//  if (::CloseHandle(m_handle) == 0) {
//    LogWarn(StringFormat("Failed to deallocate shared memory: \"%s\": %s", GetName(), GetLastWin32Error().c_str()));
//  }
//
//  LogDebug(StringFormat("Deallocated shared memory \"%s\"", GetName()));
//  if (m_file) {
//    std::fclose(m_file);
//  }
//}
//
//void SharedMemory::LogToFile(int level, std::string msg) {
//  if (!m_file) return;
//
//  std::ostringstream ss;
//
//  // Get current date-time (up to ms accuracy)
//  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
//  auto now_ms = now.time_since_epoch();
//  auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(now_ms);
//  auto tm_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now_ms - now_sec);
//
//  std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
//  struct tm* localTime = std::localtime(&currentTime);
//
//  auto timeStr = StringFormat("%02i:%02i:%02i.%03i", localTime->tm_hour, localTime->tm_min, localTime->tm_sec, tm_ms.count());
//  ss << "[" << timeStr << "]";
//
//  switch (level) {
//    case BIFROST_LOGLEVEL_DEBUG:
//      ss << " [DEBUG]";
//      break;
//    case BIFROST_LOGLEVEL_INFO:
//      ss << " [INFO]";
//      break;
//    case BIFROST_LOGLEVEL_WARN:
//      ss << " [WARN]";
//      break;
//    case BIFROST_LOGLEVEL_ERROR:
//      ss << " [ERROR]";
//      break;
//  }
//
//  ss << " [" << m_module << "]: " << msg << "\n";
//  std::fprintf(m_file, ss.str().c_str());
//  std::fflush(m_file);
//}
//
//void SharedMemory::LogDebug(std::string msg) { LogToFile(BIFROST_LOGLEVEL_DEBUG, std::move(msg)); }
//
//void SharedMemory::LogWarn(std::string msg) { LogToFile(BIFROST_LOGLEVEL_WARN, std::move(msg)); }
//
//void SharedMemory::LogError(std::string msg) { LogToFile(BIFROST_LOGLEVEL_ERROR, std::move(msg)); }
//
//}  // namespace bifrost::shared