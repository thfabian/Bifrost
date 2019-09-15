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

#include "bifrost/api/test/data/hook_dll.h"

int bifrost_add(int arg1, int arg2) { return arg1 + arg2; }

// If the module is too small MinHook can't place a jump. We just define a dummy function here to increase the image size (this is probably never gonna be a
// problem in the real world)
BIFROST_HOOK_DLL_API void __dummy__(void) {}