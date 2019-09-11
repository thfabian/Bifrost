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

#include "bifrost/api/test/data/hook_dll.h"
#include "bifrost/api/test/data/shared.h"

using namespace bifrost;

int main(int argc, const char* argv[]) {
  const char* file = argv[1];
  int arg1 = std::atoi(argv[2]);
  int arg2 = std::atoi(argv[3]);
  int sleep = std::atoi(argv[4]);

  int result1 = bifrost_add(arg1, arg2);
  WriteToFile(file, "Result=" + std::to_string(result1));

  if (sleep > 0) {
    ::Sleep(sleep);
    int result2 = bifrost_add(arg1, arg2);
    WriteToFile(file, "Result=" + std::to_string(result2));
  }
  return 0;
}