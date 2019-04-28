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
#include "bifrost/core/context.h"
#include "bifrost/core/error.h"
#include "bifrost/core/ilogger.h"
#include "bifrost/core/shared_memory.h"
#include "bifrost/core/json.h"

namespace bifrost {

std::string Context::Export() {
  if (HasLogger()) Logger().Info("Exporting context ...");

  // Write context settings to JSON
  if (HasLogger()) Logger().Debug("Writing context settings to Clipboard ...");
  Json j;
  j["sm"] = HasSharedMemory();
  if (HasSharedMemory()) {
    j["smName"] = Memory().GetName();
    j["smSize"] = Memory().GetSizeInBytes();
  }
  std::string ctxSettings = j.dump();

  // Create global memory
  const char* memData = ctxSettings.data();
  std::size_t memSize = ctxSettings.size() + 1;

  HGLOBAL hMem = NULL;
  BIFROST_ASSERT_WIN_CALL_CTX(this, (hMem = ::GlobalAlloc(GMEM_MOVEABLE, memSize)) != NULL);

  void* memPtr = nullptr;
  BIFROST_ASSERT_WIN_CALL_CTX(this, (memPtr = ::GlobalLock(hMem)) != NULL);
  std::memcpy(memPtr, memData, memSize);

  BIFROST_ASSERT_WIN_CALL_CTX(this, ::GlobalUnlock(hMem) != FALSE);

  // Copy to clipboard
  BIFROST_ASSERT_WIN_CALL_CTX(this, ::OpenClipboard(0) != FALSE);
  BIFROST_ASSERT_WIN_CALL_CTX(this, ::EmptyClipboard() != FALSE);
  BIFROST_ASSERT_WIN_CALL_CTX(this, ::SetClipboardData(CF_TEXT, hMem) != NULL);
  BIFROST_ASSERT_WIN_CALL_CTX(this, ::CloseClipboard() != FALSE);

  if (HasLogger()) Logger().Info("Successfully exported context");
  return ctxSettings;
}

std::string Context::Import(bool dryRun) { return {}; }

}  // namespace bifrost
