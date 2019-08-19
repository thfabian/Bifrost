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
#include "bifrost/core/sm_storage.h"

namespace {

using namespace bifrost;

class SharedStorageTest : public TestBaseNoSharedMemory {};

TEST_F(SharedStorageTest, InsertAndGet) {
  Context* ctx = GetContext();
  auto mem = CreateSharedMemory(1 << 20);
  ctx->SetMemory(mem.get());

  SMStorage& storage = *ctx->Memory().GetSMStorage();
  ASSERT_EQ(0, storage.Size());

  storage.InsertString(ctx, "string", "value");
  storage.InsertInt(ctx, "int", 3);
  EXPECT_STREQ("value", storage.GetString(ctx, "string").c_str());
  storage.InsertBool(ctx, "bool", true);
  EXPECT_STREQ("value", storage.GetString(ctx, "string").c_str());
  storage.InsertDouble(ctx, "double", 2.0);
  ASSERT_EQ(4, storage.Size());

  // Direct
  EXPECT_EQ(std::string_view{"value"}, storage.GetStringView(ctx, "string"));
  EXPECT_EQ(3, storage.GetInt(ctx, "int"));
  EXPECT_EQ(true, storage.GetBool(ctx, "bool"));
  EXPECT_EQ(2.0, storage.GetDouble(ctx, "double"));

  // Conversion to string
  EXPECT_STREQ("3", storage.GetString(ctx, "int").c_str());
  EXPECT_STREQ("1", storage.GetString(ctx, "bool").c_str());
  EXPECT_STREQ("2.000000", storage.GetString(ctx, "double").c_str());

  storage.InsertString(ctx, "one", "1.0");
  EXPECT_EQ(true, storage.GetBool(ctx, "one"));
  EXPECT_EQ(1, storage.GetInt(ctx, "one"));
  EXPECT_EQ(1.0, storage.GetDouble(ctx, "one"));

  // Conversion to bool
  EXPECT_THROW(storage.GetBool(ctx, "string"), std::runtime_error);
  EXPECT_EQ(true, storage.GetBool(ctx, "int"));
  EXPECT_EQ(true, storage.GetBool(ctx, "double"));

  // Conversion to int
  EXPECT_THROW(storage.GetInt(ctx, "string"), std::runtime_error);
  EXPECT_EQ(1, storage.GetInt(ctx, "bool"));
  EXPECT_EQ(2, storage.GetInt(ctx, "double"));

  // Conversion to double
  EXPECT_THROW(storage.GetDouble(ctx, "string"), std::runtime_error);
  EXPECT_EQ(1, storage.GetDouble(ctx, "bool"));
  EXPECT_EQ(3, storage.GetDouble(ctx, "int"));

  // Non-existing
  EXPECT_THROW(storage.GetInt(ctx, "unknown"), std::runtime_error);
  EXPECT_THROW(storage.GetBool(ctx, "unknown"), std::runtime_error);
  EXPECT_THROW(storage.GetDouble(ctx, "unknown"), std::runtime_error);
  EXPECT_THROW(storage.GetString(ctx, "unknown"), std::runtime_error);

  // Override
  storage.InsertString(ctx, "foo", "bar");
  EXPECT_STREQ("bar", storage.GetString(ctx, "foo").c_str());

  storage.InsertString(ctx, "foo", "bar2");
  EXPECT_STREQ("bar2", storage.GetString(ctx, "foo").c_str());
}

TEST_F(SharedStorageTest, InsertAndRemove) {
  Context* ctx = GetContext();
  auto mem = CreateSharedMemory(1 << 12);
  ctx->SetMemory(mem.get());

  SMStorage& storage = *ctx->Memory().GetSMStorage();
  ASSERT_EQ(0, storage.Size());
  auto initialMem = ctx->Memory().GetNumFreeBytes();

  // Insert and remove again -> check if all memory is properly freed
  storage.InsertInt(ctx, "foo", 5);

  ASSERT_EQ(1, storage.Size());
  ASSERT_TRUE(storage.Remove(ctx, "foo"));
  ASSERT_FALSE(storage.Remove(ctx, "foo"));
  ASSERT_EQ(0, storage.Size());
  storage.Clear(ctx);

  EXPECT_EQ(initialMem, ctx->Memory().GetNumFreeBytes());
}

}  // namespace