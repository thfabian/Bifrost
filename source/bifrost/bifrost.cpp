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

#include "bifrost/bifrost.h"
#include "bifrost_core/error.h"
#include "bifrost_core/macros.h"

using namespace bifrost;

#pragma region Error

extern BIFROST_API const char* bifrost_GetLastError() BIFROST_NOEXCEPT { return Error::Get().GetLastError(); }

#pragma endregion

#pragma region Version

extern BIFROST_API const char* bifrost_GetVersion() BIFROST_NOEXCEPT {
  return BIFROST_STRINGIFY(BIFROST_VERSION_MAJOR) "." BIFROST_STRINGIFY(BIFROST_VERSION_MINOR) "." BIFROST_STRINGIFY(BIFROST_VERSION_PATCH);
}

#pragma endregion

#pragma region DllMain

extern "C" BOOL APIENTRY DllMain(HMODULE hModule, DWORD ulReasonForCall, LPVOID lpReserved) {
  switch (ulReasonForCall) {
    case DLL_PROCESS_ATTACH:
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
      break;
  }
  return TRUE;
}

#pragma endregion
