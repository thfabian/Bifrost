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

#include "bifrost/core/test/test.h"
#include "bifrost/core/sm_string.h"

namespace {

using namespace bifrost;

class SMStringTest : public TestBaseSharedMemory {};

TEST_F(SMStringTest, ValueConstructor) {
  auto ctx = GetContext();

  SMString s1(ctx);
  EXPECT_EQ(0, s1.Size());

  SMString s2(ctx, 10);
  EXPECT_EQ(10, s2.Size());

  auto str3 = "Hell world!";
  SMString s3(ctx, str3);
  EXPECT_EQ(std::strlen(str3), s3.AsView(ctx).size());
  EXPECT_EQ(std::strlen(str3), s3.AsString(ctx).size());
  EXPECT_STREQ(str3, s3.AsString(ctx).c_str());

  auto str4 = std::string("Hello World!");
  SMString s4(ctx, str4);
  EXPECT_EQ(str4.size(), s4.AsView(ctx).size());
  EXPECT_EQ(str4.size(), s4.AsString(ctx).size());
  EXPECT_STREQ(str4.c_str(), s4.AsString(ctx).c_str());
}

TEST_F(SMStringTest, MoveConstructor) {
  auto ctx = GetContext();

  auto str = std::string("Hello World!");
  SMString s1(ctx, str);
  SMString s2(std::move(s1));

  EXPECT_EQ(0, s1.Size());
  EXPECT_EQ(str.size(), s2.Size());
  EXPECT_STREQ(str.c_str(), s2.AsString(ctx).c_str());
}

TEST_F(SMStringTest, MoveAssign) {
  auto ctx = GetContext();

  auto str = std::string("Hello World!");
  SMString s1(ctx, str);
  SMString s2(ctx);
  s2 = std::move(s1);

  EXPECT_EQ(0, s1.Size());
  EXPECT_EQ(str.size(), s2.Size());
  EXPECT_STREQ(str.c_str(), s2.AsString(ctx).c_str());
}

TEST_F(SMStringTest, CopyAssign) {
  auto ctx = GetContext();

  auto str = std::string("Hello World!");
  SMString s1(ctx, str);
  SMString s2(ctx);
  s2.Assign(ctx, s1);

  EXPECT_EQ(str.size(), s1.Size());
  EXPECT_EQ(str.size(), s2.Size());
  EXPECT_STREQ(str.c_str(), s1.AsString(ctx).c_str());
  EXPECT_STREQ(str.c_str(), s2.AsString(ctx).c_str());
}

}  // namespace