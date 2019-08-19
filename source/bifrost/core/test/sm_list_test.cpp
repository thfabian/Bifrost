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
#include "bifrost/core/sm_list.h"

namespace {

using namespace bifrost;

class SMListTest : public TestBaseSharedMemory {};

TEST_F(SMListTest, Construction) {
  auto ctx = GetContext();

  SMList<i32> list;
  EXPECT_EQ(0, list.Size(ctx));
  EXPECT_TRUE(list.Empty());
}

TEST_F(SMListTest, Destruction) {}

TEST_F(SMListTest, PushPopFront) {
  auto ctx = GetContext();

  SMList<i32> list;
  i32* headValue = nullptr;
  i32* tailValue = nullptr;

  list.PushFront(ctx, 42);
  EXPECT_FALSE(list.Empty());
  EXPECT_EQ(1, list.Size(ctx));

  headValue = list.PeekFront(ctx);
  ASSERT_NE(nullptr, headValue);
  EXPECT_EQ(42, *headValue);

  tailValue = list.PeekBack(ctx);
  ASSERT_NE(nullptr, tailValue);
  EXPECT_EQ(42, *tailValue);

  list.PushFront(ctx, 43);
  list.PushFront(ctx, 44);
  EXPECT_FALSE(list.Empty());
  EXPECT_EQ(3, list.Size(ctx));

  headValue = list.PeekFront(ctx);
  ASSERT_NE(nullptr, headValue);
  EXPECT_EQ(44, *headValue);

  tailValue = list.PeekBack(ctx);
  ASSERT_NE(nullptr, tailValue);
  EXPECT_EQ(42, *tailValue);

  list.PopFront(ctx);
  headValue = list.PeekFront(ctx);
  ASSERT_NE(nullptr, headValue);
  EXPECT_EQ(43, *headValue);

  tailValue = list.PeekBack(ctx);
  ASSERT_NE(nullptr, tailValue);
  EXPECT_EQ(42, *tailValue);
}

TEST_F(SMListTest, PushPopBack) {
  auto ctx = GetContext();

  SMList<i32> list;
  i32* headValue = nullptr;
  i32* tailValue = nullptr;

  list.PushBack(ctx, 42);
  EXPECT_FALSE(list.Empty());
  EXPECT_EQ(1, list.Size(ctx));

  headValue = list.PeekFront(ctx);
  ASSERT_NE(nullptr, headValue);
  EXPECT_EQ(42, *headValue);

  tailValue = list.PeekBack(ctx);
  ASSERT_NE(nullptr, tailValue);
  EXPECT_EQ(42, *tailValue);

  list.PushBack(ctx, 43);
  list.PushBack(ctx, 44);
  EXPECT_FALSE(list.Empty());
  EXPECT_EQ(3, list.Size(ctx));

  headValue = list.PeekFront(ctx);
  ASSERT_NE(nullptr, headValue);
  EXPECT_EQ(42, *headValue);

  tailValue = list.PeekBack(ctx);
  ASSERT_NE(nullptr, tailValue);
  EXPECT_EQ(44, *tailValue);

  list.PopBack(ctx);
  headValue = list.PeekFront(ctx);
  ASSERT_NE(nullptr, headValue);
  EXPECT_EQ(42, *headValue);

  tailValue = list.PeekBack(ctx);
  ASSERT_NE(nullptr, tailValue);
  EXPECT_EQ(43, *tailValue);
}

}  // namespace
