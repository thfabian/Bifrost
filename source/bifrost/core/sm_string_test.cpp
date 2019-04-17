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
#include "bifrost/core/sm_string.h"

namespace {

using namespace bifrost;

class SMStringTest : public TestBase<true> {};

TEST_F(SMStringTest, ValueConstructor) {
  SMString s1(GetContext());
  EXPECT_EQ(0, s1.Size());

  SMString s2(GetContext(), 10);
  EXPECT_EQ(10, s2.Size());

  auto str3 = "Hell world!";
  SMString s3(GetContext(), str3);
  EXPECT_EQ(std::strlen(str3), s3.AsView().size());
  EXPECT_EQ(std::strlen(str3), s3.AsString().size());
  EXPECT_STREQ(str3, s3.AsString().c_str());

  auto str4 = std::string("Hello World!");
  SMString s4(GetContext(), str4);
  EXPECT_EQ(str4.size(), s4.AsView().size());
  EXPECT_EQ(str4.size(), s4.AsString().size());
  EXPECT_STREQ(str4.c_str(), s4.AsString().c_str());
}

TEST_F(SMStringTest, MoveConstructor) {
  auto str = std::string("Hello World!");
  SMString s1(GetContext(), str);
  SMString s2(std::move(s1));

  EXPECT_NE(s1, s2);
  EXPECT_EQ(0, s1.Size());
  EXPECT_EQ(str.size(), s2.Size());
  EXPECT_STREQ(str.c_str(), s2.AsString().c_str());
}

TEST_F(SMStringTest, MoveAssign) {
  auto str = std::string("Hello World!");
  SMString s1(GetContext(), str);
  SMString s2(GetContext());
  s2 = std::move(s1);

  EXPECT_NE(s1, s2);
  EXPECT_EQ(0, s1.Size());
  EXPECT_EQ(str.size(), s2.Size());
  EXPECT_STREQ(str.c_str(), s2.AsString().c_str());
}

TEST_F(SMStringTest, CopyConstructor) {
  auto str = std::string("Hello World!");
  SMString s1(GetContext(), str);
  SMString s2(s1);

  EXPECT_EQ(s1, s2);
  EXPECT_EQ(str.size(), s1.Size());
  EXPECT_EQ(str.size(), s2.Size());
  EXPECT_STREQ(str.c_str(), s1.AsString().c_str());
  EXPECT_STREQ(str.c_str(), s2.AsString().c_str());
}

TEST_F(SMStringTest, CopyAssign) {
  auto str = std::string("Hello World!");
  SMString s1(GetContext(), str);
  SMString s2(GetContext());
  s2 = s1;

  EXPECT_EQ(s1, s2);
  EXPECT_EQ(str.size(), s1.Size());
  EXPECT_EQ(str.size(), s2.Size());
  EXPECT_STREQ(str.c_str(), s1.AsString().c_str());
  EXPECT_STREQ(str.c_str(), s2.AsString().c_str());
}

}  // namespace