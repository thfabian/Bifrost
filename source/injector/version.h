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

#include "bifrost/core/macros.h"

/// Major version
#define INJECTOR_VERSION_MAJOR 0

/// Minor version [0-99]
#define INJECTOR_VERSION_MINOR 0

/// Patch version [0-99]
#define INJECTOR_VERSION_PATCH 1

#define INJECTOR_VERSION_STRING \
  BIFROST_STRINGIFY(INJECTOR_VERSION_MAJOR) "." BIFROST_STRINGIFY(INJECTOR_VERSION_MINOR) "." BIFROST_STRINGIFY(INJECTOR_VERSION_PATCH)