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

#include "saxpy.h"

#include <vector>

int main(int argc, const char* argv[]) {
  int n = argc > 1 ? std::atoi(argv[1]) : 1 << 20;

  // Allocate the two vectors
  std::vector<float> x(n, 1.0f);
  std::vector<float> y(n, 1.0f);
  float a = 3.0f;

  // Call saxpy
  saxpy(n, a, x.data(), y.data());

  // Check the result
  int numFailed = 0;
  for (int i = 0; i < n; ++i) {
    if (y[i] != 4.0f) numFailed++;
  }

  return numFailed == 0 ? 0 : 1;
}