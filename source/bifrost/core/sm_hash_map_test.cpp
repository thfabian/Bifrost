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
#include "bifrost/core/sm_hash_map.h"
#include "bifrost/core/sm_string.h"

namespace {

using namespace bifrost;

class SMHashMapTest : public TestBase<true> {};

TEST_F(SMHashMapTest, ConstructionI32) {
  auto ctx = GetContext();

  SMHashMap<i32, i32> map1(ctx, 1);
  EXPECT_EQ(0, map1.Size());
  EXPECT_EQ(1, map1.Capacity());

  SMHashMap<i32, i32> map2(ctx, 10);
  EXPECT_EQ(0, map2.Size());
  EXPECT_EQ(10, map2.Capacity());
}

TEST_F(SMHashMapTest, InsertI32) {
  auto ctx = GetContext();

  for (auto initialCap : {1, 16}) {
    SMHashMap<i32, i32> map(ctx, initialCap);
    EXPECT_EQ(0, map.Size());

    auto ptr = map.Insert(ctx, 0, 2);
    EXPECT_EQ(1, map.Size());
    EXPECT_EQ(0, ptr->Key);
    EXPECT_EQ(2, ptr->Value);

    ASSERT_NE(nullptr, map.Get(ctx, 0));
    EXPECT_EQ(2, *map.Get(ctx, 0));

    ptr = map.Insert(ctx, 1, 2);
    EXPECT_EQ(2, map.Size());
    EXPECT_EQ(1, ptr->Key);
    EXPECT_EQ(2, ptr->Value);

    ASSERT_NE(nullptr, map.Get(ctx, 1));
    EXPECT_EQ(2, *map.Get(ctx, 1));

    if (initialCap != 1) {
      EXPECT_EQ(ptr, map.Insert(ctx, 1, 2));
    }
  }
}

TEST_F(SMHashMapTest, InsertString) {
  auto ctx = GetContext();
  auto initialMem = ctx->Memory().GetNumFreeBytes();

  SMHashMap<SMString, SMString> map;

  SMString key1{ctx, "foo1"};
  SMString value1{ctx, "bar1"};
  map.Insert(ctx, key1, std::move(value1));
  key1.Clear(ctx);
  EXPECT_EQ(1, map.Size());

  SMString key2{ctx, "foo2"};
  SMString value2{ctx, "bar2"};
  map.Insert(ctx, key2, std::move(value2));
  key2.Clear(ctx);
  EXPECT_EQ(2, map.Size());

  map.Clear(ctx);
  EXPECT_EQ(0, map.Size());

  EXPECT_EQ(initialMem, ctx->Memory().GetNumFreeBytes());
}

TEST_F(SMHashMapTest, RemoveI32) {
  auto ctx = GetContext();

  SMHashMap<i32, i32> map;
  EXPECT_EQ(0, map.Size());

  for (auto i : {1, 2, 3}) {
    map.Insert(ctx, i, i);
    EXPECT_NE(nullptr, map.Get(ctx, i));
  }
  EXPECT_EQ(3, map.Size());

  EXPECT_NO_THROW(map.Remove(ctx, 3));
  EXPECT_EQ(2, map.Size());
  EXPECT_EQ(nullptr, map.Get(ctx, 3));

  for (auto i : {1, 2, 3}) {
    EXPECT_NO_THROW(map.Remove(ctx, i));
  }
  EXPECT_EQ(0, map.Size());
}

}  // namespace
