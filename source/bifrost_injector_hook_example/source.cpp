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

#include "header.h"
#include <iostream>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int foo(int bar) { return bar + 1; }

int main(int argc, const char* argv) {
  int bar = 5;
  std::cout << "bar = " << bar << std::endl;
  bar = foo(bar);
  Sleep(500);
  std::cout << "bar = " << bar << std::endl;
}