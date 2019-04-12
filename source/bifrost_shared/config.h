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

//
// FreeList
//
#define BIFROST_MALLOC_FREELIST_BLOCKSIZE 64

//
// Shared Memory
//
#define BIFROST_SHARED_MEMORY_NAME "__bifrost_shared__"
#define BIFROST_SHARED_MEMORY_SIZE 1 << 25  // 32 MB

//
// Log Stash
//
#define BIFROST_LOG_STASH_BLOCK_SIZE 1 << 10  // 1 MB