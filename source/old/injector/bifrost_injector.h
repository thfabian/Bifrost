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

/// @file Bifrost injector allows to inject bifrost_loader.dll into any executable.

#pragma region Macros

/// @brief Major version of Bifrost
#define BIFROST_INJECTOR_VERSION_MAJOR 0

/// @brief Minor version [0-99]
#define BIFROST_INJECTOR_VERSION_MINOR 0

/// @brief Patch version [0-99]
#define BIFROST_INJECTOR_VERSION_PATCH 1

/// @brief Integer of the version
#define BIFROST_INJECTOR_VERSION (100 * (100 * BIFROST_INJECTOR_VERSION_MAJOR + BIFROST_INJECTOR_VERSION_MINOR) + BIFROST_INJECTOR_VERSION_PATCH)

#ifdef BIFROST_INJECTOR_EXPORTS
#define BIFROST_INJECTOR_API __declspec(dllexport)
#else
#define BIFROST_INJECTOR_API __declspec(dllimport)
#endif

/// @brief Prefix for shared storage keys
#define BFI(Key) "bfi." Key

#pragma endregion

#if __cplusplus
extern "C" {
#endif

#pragma region Version

/// @brief Get the version string
/// @remarks
///    This function is thread-safe.
BIFROST_INJECTOR_API const char* bfi_GetVersion();

#pragma endregion

#pragma region Injector

/// @brief ErrorStash status
enum bfi_Status {
  BFI_OK = 0,  ///< Everything is fine - no error
  BFI_FAIL,    ///< An error occurred
};

/// @brief Injector context
struct bfi_InjectorContext_t {
  void* Ctx;  ///< Context object
};
typedef bfi_InjectorContext_t bfi_InjectorContext;

/// @brief Convert the status to string
BIFROST_INJECTOR_API const char* bfi_InjectorGetLastError(bfi_InjectorContext* ctx);

/// @brief
struct bfi_InjectorDesc {
  const char* foo;
};

/// @brief Initialize the injector
BIFROST_INJECTOR_API bfi_Status bfi_InjectorInit(bfi_InjectorContext* ctx, bfi_InjectorDesc* desc);

/// @brief Perform the injection
BIFROST_INJECTOR_API bfi_Status bfi_InjectorInjectAndWait();

/// @brief Clean-up the injector
BIFROST_INJECTOR_API bfi_Status bfi_InjectorFinalize(bfi_InjectorContext* ctx);

#pragma endregion

#if __cplusplus
}  // extern "C"
#endif
