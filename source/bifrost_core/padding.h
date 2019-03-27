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

#include "bifrost_core/common.h"

namespace bifrost {

#pragma pack(push)
#pragma pack(1)

template <std::size_t SizeInBytes>
struct Padding {
  u8 Data[SizeInBytes];
};

template <>
struct Padding<0> {};

#pragma pack(pop)

}  // namespace bifrost
