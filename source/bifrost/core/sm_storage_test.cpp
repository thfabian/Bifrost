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
#include "bifrost/core/sm_storage.h"

namespace {

using namespace bifrost;

class SharedStorageTest : public TestBase<true> {};

//TEST_F(SharedStorageTest, InsertString) {
//  Context* ctx = GetContext();
//  SMStorage& storage = *ctx->Memory().GetSMStorage();
//  ASSERT_EQ(0, storage.Size());
//
//  SMString key{ctx, "key"};
//  storage.Insert(ctx, key, SMStorageValue::FromString(ctx, "value"));
//  ASSERT_EQ(1, storage.Size());
//  ASSERT_NO_THROW(storage.GetOrThrow(ctx, key));
//
//  // As string
//  EXPECT_STREQ("value", storage.GetOrThrow(ctx, key).AsString(ctx).c_str());
//  EXPECT_EQ(std::string_view{"value"}, storage.GetOrThrow(ctx, key).AsStringView(ctx));
//
//  ASSERT_ANY_THROW(storage.GetOrThrow(ctx, key).AsBool(ctx));
//  ASSERT_ANY_THROW(storage.GetOrThrow(ctx, key).AsInt(ctx));
//  ASSERT_ANY_THROW(storage.GetOrThrow(ctx, key).AsDouble(ctx));
//}
//
//TEST_F(SharedStorageTest, InsertBool) {
//  Context* ctx = GetContext();
//  SMStorage& storage = *ctx->Memory().GetSMStorage();
//  ASSERT_EQ(0, storage.Size());
//
//  SMString key{ctx, "key"};
//  storage.Insert(ctx, key, SMStorageValue::FromBool(ctx, true));
//  ASSERT_EQ(1, storage.Size());
//  ASSERT_NO_THROW(storage.GetOrThrow(ctx, key));
//
//  EXPECT_EQ(true, storage.GetOrThrow(ctx, key).AsBool(ctx));
//  EXPECT_EQ(1, storage.GetOrThrow(ctx, key).AsInt(ctx));
//  EXPECT_EQ(1.0, storage.GetOrThrow(ctx, key).AsDouble(ctx));
//  EXPECT_STREQ("1", storage.GetOrThrow(ctx, key).AsString(ctx).c_str());
//
//  ASSERT_ANY_THROW(storage.GetOrThrow(ctx, key).AsStringView(ctx));
//}

}  // namespace