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

#include "hello_world.h"

#include <Windows.h>
#include <iostream>

int main(int argc, const char* argv[]) {
  ::Sleep(argc > 1 ? std::atoi(argv[1]) : 3000);

  const int a = 1;
  const int b = 2;
  return ::hello_world_add(a, b) == (a + b);
}