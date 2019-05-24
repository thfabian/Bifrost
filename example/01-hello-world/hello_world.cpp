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

#include <windows.h>
#include <iostream>

int hello_world(int bar) { return bar + 1; }

int main(int argc, const char* argv[]) {
  int bar = hello_world(argc > 1 ? std::atoi(argv[1]) : -1);
  ::Sleep(2000);
  return bar;
}