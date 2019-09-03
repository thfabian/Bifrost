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

/// saxpy stands for "single-precision a·x plus y"
///
/// It is a function in the standard Basic Linear Algebra Subroutines (BLAS). saxpy is a combination of scalar multiplication and vector addition, and its very
/// simple: it takes as input two vectors of 32-bit floats `x` and `y` with `n` elements each, and a scalar value `a`. It multiplies each element `x[i]` by `a`
/// and adds the result to `y[i]`.
extern "C" __declspec(dllexport) void saxpy(int n, float a, float* x, float* y);
