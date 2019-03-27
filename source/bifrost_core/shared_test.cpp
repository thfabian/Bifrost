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

#include "bifrost_core/shared.h"
#include <gtest/gtest.h>

namespace {

using namespace bifrost;
TEST(Shared, Bool) {
  Shared::Get().WriteBool("test.bool", true);
  EXPECT_EQ(true, Shared::Get().ReadBool("test.bool"));
  EXPECT_THROW(Shared::Get().ReadBool("test.boolX"), std::runtime_error);
  EXPECT_EQ(true, Shared::Get().ReadBool("test.bool", false));
  EXPECT_EQ(false, Shared::Get().ReadBool("test.boolX", false));
}

TEST(Shared, Int) {
  //Shared::Get().WriteInt("test.int", 42);
  //EXPECT_EQ(42, Shared::Get().ReadInt("test.int"));
  //EXPECT_THROW(Shared::Get().ReadInt("test.intX"), std::runtime_error);
  //EXPECT_EQ(42, Shared::Get().ReadInt("test.int", 1));
  //EXPECT_EQ(1, Shared::Get().ReadInt("test.intX", 1));
}

}  // namespace
