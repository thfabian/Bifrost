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

#include "bifrost/core/test.h"
#include "bifrost/core/sm_list.h"

namespace {

using namespace bifrost;

class SMListTest : public TestBase<true> {};

TEST_F(SMListTest, Construction) {
  // SMHashMap<i32, i32> map1(GetContext(), 1);
  // EXPECT_EQ(0, map1.Size());
  // EXPECT_EQ(1, map1.Capacity());

  // SMHashMap<i32, i32> map2(GetContext(), 10);
  // EXPECT_EQ(0, map2.Size());
  // EXPECT_EQ(10, map2.Capacity());
}

}  // namespace
