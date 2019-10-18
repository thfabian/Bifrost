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

#ifdef BIFROST_HOOK_DLL_EXPORTS
#define BIFROST_HOOK_DLL_API __declspec(dllexport)
#else
#define BIFROST_HOOK_DLL_API __declspec(dllimport)
#endif

//
// C-Function
//
extern "C" {
BIFROST_HOOK_DLL_API int bifrost_add(int arg1, int arg2);
}

//
// Class method
//
namespace bifrost {

class BIFROST_HOOK_DLL_API Adder {
 public:
  int add(int arg1, int arg2);
};

}  // namespace bifrost

//
// VTable hook
//
namespace bifrost {}