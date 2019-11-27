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

namespace bifrost {

enum class Mode : int {
  none = 0,
  CFunction_Single_Original1,
  CFunction_Single_Original2,
  CFunction_Single_Original3,
  CFunction_Single_Modify1,
  CFunction_Single_Modify2,
  CFunction_Single_Modify3,
  CFunction_Single_Replace1,
  CFunction_Single_Restore1,

  CFunction_Multi_Original_P1,
  CFunction_Multi_Original_P2,
  CFunction_Multi_Original_P3,
  CFunction_Multi_Modify1_P1,
  CFunction_Multi_Modify1_P2,
  CFunction_Multi_Modify2_P1,
  CFunction_Multi_Modify2_P2,
  CFunction_Multi_Modify3_P1,
  CFunction_Multi_Modify3_P2,
  CFunction_Multi_Modify4_P1,
  CFunction_Multi_Modify4_P2,
  CFunction_Multi_Modify4_P3,
  CFunction_Multi_Modify5_P1,
  CFunction_Multi_Modify5_P2,
  CFunction_Multi_Modify5_P3,

  VTable_Single_Original1,
  VTable_Single_Original2,
  VTable_Single_Original3,
  VTable_Single_Original4,
  VTable_Single_Modify1,
  VTable_Single_Modify2,
  VTable_Single_Modify3,
  VTable_Single_Replace1,
  VTable_Single_Restore1,
};

}