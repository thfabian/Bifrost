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
#include <Windows.h>

void saxpy(int n, float a, float* x, float* y) {
  // Oh no.. there is a bug here!
  int upper_bound = n / 2;
  for (int i = 0; i < upper_bound; ++i) y[i] = a * x[i] + y[i];
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) { return TRUE; }