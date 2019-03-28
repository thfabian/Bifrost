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
  EXPECT_STRCASEEQ("1", Shared::Get().ReadString("test.bool").data());
}

TEST(Shared, Int) {
  Shared::Get().WriteInt("test.int", 42);

  EXPECT_EQ(42, Shared::Get().ReadInt("test.int"));
  EXPECT_THROW(Shared::Get().ReadInt("test.intX"), std::runtime_error);
  EXPECT_EQ(42, Shared::Get().ReadInt("test.int", 1));
  EXPECT_EQ(1, Shared::Get().ReadInt("test.intX", 1));
  EXPECT_STRCASEEQ("42", Shared::Get().ReadString("test.int").data());

  Shared::Get().WriteString("test.intStr", "84");
  Shared::Get().WriteString("test.intStrX", "X");

  EXPECT_EQ(84, Shared::Get().ReadInt("test.intStr"));
  EXPECT_THROW(Shared::Get().ReadInt("test.intStrX"), std::runtime_error);
}

TEST(Shared, Double) {
  Shared::Get().WriteDouble("test.double", 0.25);

  EXPECT_EQ(0.25, Shared::Get().ReadDouble("test.double"));
  EXPECT_THROW(Shared::Get().ReadDouble("test.doubleX"), std::runtime_error);
  EXPECT_EQ(0.25, Shared::Get().ReadDouble("test.double", 1.25));
  EXPECT_EQ(1.25, Shared::Get().ReadDouble("test.doubleX", 1.25));
  EXPECT_STRCASEEQ("0.250000", Shared::Get().ReadString("test.double").data());

  Shared::Get().WriteString("test.doubleStr", "84.25");
  Shared::Get().WriteString("test.doubleStrX", "X");

  EXPECT_EQ(84.25, Shared::Get().ReadDouble("test.doubleStr"));
  EXPECT_THROW(Shared::Get().ReadDouble("test.doubleStrX"), std::runtime_error);
}

}  // namespace
