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

#include "bifrost/core/common.h"
#include "bifrost/core/context.h"

namespace bifrost {

template <class T>
struct SMHasher : public std::hash<T> {
  SMHasher(Context* ctx) {}
};

template <class T>
struct SMEqualTo : public std::equal_to<T> {
  SMEqualTo(Context* ctx) {}
};

template <class T>
struct SMAssign {
  void operator()(T& left, const T& rigth) const { left = rigth; }
  SMAssign(Context* ctx) {}
};

}  // namespace bifrost