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
  CFunction_Single_Orignal1,
  CFunction_Single_Orignal2,
  CFunction_Single_Orignal3,
  CFunction_Single_Modify1,
  CFunction_Single_Modify2,
  CFunction_Single_Modify3,
  CFunction_Single_Replace1,
};

}