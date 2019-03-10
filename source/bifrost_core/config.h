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
// Logging
//
#define BIFROST_LOGLEVEL_DEBUG 0
#define BIFROST_LOGLEVEL_INFO 1
#define BIFROST_LOGLEVEL_WARN 2
#define BIFROST_LOGLEVEL_ERROR 3
#define BIFROST_LOGLEVEL_DISABLE 4

#ifndef BIFROST_LOGLEVEL
#define BIFROST_LOGLEVEL BIFROST_LOGLEVEL_DEBUG
#endif