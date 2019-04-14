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
#include "bifrost/core/ptr.h"

namespace {

using namespace bifrost;

TEST(PtrTest, Comparison) {
  Ptr<i32> p0;
  Ptr<i32> p1(1);
  Ptr<i32> p2(2);
  Ptr<i32> p1_clone = p1;

  EXPECT_EQ(0, p0.Offset());
  EXPECT_EQ(1, p1.Offset());
  EXPECT_EQ(2, p2.Offset());

  void* base_addr = (void*)100;
  EXPECT_EQ(100, (u64)p0.Resolve(base_addr));
  EXPECT_EQ(101, (u64)p1.Resolve(base_addr));
  EXPECT_EQ(102, (u64)p2.Resolve(base_addr));

  // Comparison
  EXPECT_FALSE(p1 == p2);
  EXPECT_TRUE(p1 != p2);

  EXPECT_TRUE(p1 == p1_clone);
  EXPECT_FALSE(p1 != p1_clone);

  EXPECT_LE(p1, p1);
  EXPECT_LE(p1, p2);
  EXPECT_LT(p1, p2);
  EXPECT_GT(p2, p1);
  EXPECT_GE(p2, p1);
  EXPECT_GE(p2, p2);
}

TEST(PtrTest, Cast) {
  Ptr<i32> p1(1);
  Ptr<i64> p2(1);

  Ptr<i64> p3 = p1.Cast<i64>();
  EXPECT_EQ(p1.Offset(), p3.Offset());
  EXPECT_EQ(p2.Offset(), p3.Offset());
}

TEST(PtrTest, Modification) {
  Ptr<i8> p1(10);
  Ptr<i32> p2(10);
  Ptr<i64> p3(10);

  EXPECT_EQ(Ptr<i8>(11), p1 + 1);
  EXPECT_EQ(Ptr<i32>(14), p2 + 1);
  EXPECT_EQ(Ptr<i64>(18), p3 + 1);

  EXPECT_EQ(Ptr<i8>(9), p1 - 1);
  EXPECT_EQ(Ptr<i32>(6), p2 - 1);
  EXPECT_EQ(Ptr<i64>(2), p3 - 1);

  EXPECT_EQ(Ptr<i8>(11), p1 += 1);
  EXPECT_EQ(Ptr<i32>(14), p2 += 1);
  EXPECT_EQ(Ptr<i64>(18), p3 += 1);

  EXPECT_EQ(Ptr<i8>(10), p1 -= 1);
  EXPECT_EQ(Ptr<i32>(10), p2 -= 1);
  EXPECT_EQ(Ptr<i64>(10), p3 -= 1);
}

}  // namespace