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

//
// This dll implements the functionality which can be hooked by the test plugins (source/bifrost/api/test/data/hook_plugin_1.cpp). It is called in the hook
// executable (source/bifrost/api/test/data/hook_executable.cpp)
//

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
// VTable method
//
namespace bifrost {

class BIFROST_HOOK_DLL_API IAdder {
 public:
  int virtual add(int arg1, int arg2) = 0;
};

class BIFROST_HOOK_DLL_API Adder : IAdder {
 public:
  int virtual add(int arg1, int arg2) override;
};

}  // namespace bifrost

//
// VTable hook
//
namespace bifrost {}