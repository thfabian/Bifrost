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

#include "bifrost_injector/common.h"
#include "bifrost_injector/injector.h"
#include "bifrost/core/error.h"
#include "bifrost/core/macros.h"
#include "bifrost/core/logging.h"

using namespace bifrost;
using namespace bifrost::injector;
//
//#define BIFROST_INJECTOR_CATCH_ALL(stmts) \
//  try {                                 \
//    stmts;                              \
//  } catch (std::exception& e) {           \
//    ;                 \
//  }
//
//BIFROST_INJECTOR_API const char* bfi_GetVersion() {
//  return BIFROST_STRINGIFY(BIFROST_INJECTOR_VERSION_MAJOR) "." BIFROST_STRINGIFY(BIFROST_INJECTOR_VERSION_MINOR) "." BIFROST_STRINGIFY(
//      BIFROST_INJECTOR_VERSION_PATCH);
//}
//
//BIFROST_INJECTOR_API const char* bfi_GetVersion() {}
