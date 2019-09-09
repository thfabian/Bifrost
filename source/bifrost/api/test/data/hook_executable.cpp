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

#include <Windows.h>
#include <cstdlib>

#include "shared.h"

int main(int argc, const char* argv[]) {
  ::Sleep(argc > 2 ? std::atoi(argv[2]) : 1000);
  return argc > 1 ? std::atoi(argv[1]) : 0;
}