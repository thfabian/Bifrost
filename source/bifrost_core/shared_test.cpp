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

#include "bifrost_core/api_shared.h"
#include <gtest/gtest.h>

namespace {

using namespace bifrost::api;

TEST(ApiShared, Bool) {
  ApiShared::Get().WriteBool("test.bool", true);
  EXPECT_EQ(true, ApiShared::Get().ReadBool("test.bool"));
  EXPECT_THROW(ApiShared::Get().ReadBool("test.boolX"), std::runtime_error);
  EXPECT_EQ(true, ApiShared::Get().ReadBool("test.bool", false));
  EXPECT_EQ(false, ApiShared::Get().ReadBool("test.boolX", false));
  EXPECT_STRCASEEQ("1", ApiShared::Get().ReadString("test.bool").data());
}

TEST(ApiShared, Int) {
  ApiShared::Get().WriteInt("test.int", 42);

  EXPECT_EQ(42, ApiShared::Get().ReadInt("test.int"));
  EXPECT_THROW(ApiShared::Get().ReadInt("test.intX"), std::runtime_error);
  EXPECT_EQ(42, ApiShared::Get().ReadInt("test.int", 1));
  EXPECT_EQ(1, ApiShared::Get().ReadInt("test.intX", 1));
  EXPECT_STRCASEEQ("42", ApiShared::Get().ReadString("test.int").c_str());

  ApiShared::Get().WriteString("test.intStr", "84");
  ApiShared::Get().WriteString("test.intStrX", "X");

  EXPECT_EQ(84, ApiShared::Get().ReadInt("test.intStr"));
  EXPECT_THROW(ApiShared::Get().ReadInt("test.intStrX"), std::runtime_error);
}

TEST(ApiShared, Double) {
  ApiShared::Get().WriteDouble("test.double", 0.25);

  EXPECT_EQ(0.25, ApiShared::Get().ReadDouble("test.double"));
  EXPECT_THROW(ApiShared::Get().ReadDouble("test.doubleX"), std::runtime_error);
  EXPECT_EQ(0.25, ApiShared::Get().ReadDouble("test.double", 1.25));
  EXPECT_EQ(1.25, ApiShared::Get().ReadDouble("test.doubleX", 1.25));
  EXPECT_STRCASEEQ("0.250000", ApiShared::Get().ReadString("test.double").c_str());

  ApiShared::Get().WriteString("test.doubleStr", "84.25");
  ApiShared::Get().WriteString("test.doubleStrX", "X");

  EXPECT_EQ(84.25, ApiShared::Get().ReadDouble("test.doubleStr"));
  EXPECT_THROW(ApiShared::Get().ReadDouble("test.doubleStrX"), std::runtime_error);
}

TEST(ApiShared, String) {
  ApiShared::Get().WriteString("test.string", "foo");

  EXPECT_STRCASEEQ("foo", ApiShared::Get().ReadString("test.string").c_str());
  EXPECT_THROW(ApiShared::Get().ReadString("test.stringX"), std::runtime_error);
  EXPECT_EQ("foo", ApiShared::Get().ReadString("test.string", "bar"));
  EXPECT_EQ("bar", ApiShared::Get().ReadString("test.stringX", "bar"));
}

}  // namespace
