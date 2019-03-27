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

#include "bifrost_shared/bifrost_shared.h"
#include <gtest/gtest.h>

namespace {

TEST(BifrostShared, Boolean) {
  bool testValue = true;

  bfs_Value valueToWrite{BFS_BOOL, testValue, sizeof(bool)};
  bfs_Write("test_int", &valueToWrite);

  bfs_Value valueToRead;
  bfs_Read("test_int", &valueToRead);

  EXPECT_EQ(valueToWrite.Type, valueToRead.Type);
  EXPECT_EQ(valueToWrite.Value, valueToRead.Value);
  EXPECT_EQ(valueToWrite.SizeInBytes, valueToRead.SizeInBytes);
}

TEST(BifrostShared, Integer) {
  int testValue = 42;

  bfs_Value valueToWrite{BFS_INT, (uint64_t)testValue, sizeof(int)};
  bfs_Write("test_bool", &valueToWrite);

  bfs_Value valueToRead;
  bfs_Read("test_bool", &valueToRead);

  EXPECT_EQ(valueToWrite.Type, valueToRead.Type);
  EXPECT_EQ(valueToWrite.Value, valueToRead.Value);
  EXPECT_EQ(valueToWrite.SizeInBytes, valueToRead.SizeInBytes);
}

TEST(BifrostShared, Double) {
  double testValue = 42;

  bfs_Value valueToWrite{BFS_DOUBLE, (uint64_t)testValue, sizeof(double)};
  bfs_Write("test_double", &valueToWrite);

  bfs_Value valueToRead;
  bfs_Read("test_double", &valueToRead);

  EXPECT_EQ(valueToWrite.Type, valueToRead.Type);
  EXPECT_EQ(valueToWrite.Value, valueToRead.Value);
  EXPECT_EQ(valueToWrite.SizeInBytes, valueToRead.SizeInBytes);
}

TEST(BifrostShared, StringInPlace) {
  std::string str = "test_string";

  bfs_Value valueToWrite{BFS_STRING, (uint64_t)str.data(), (uint32_t)str.size()};
  bfs_Write("test_str", &valueToWrite);

  bfs_Value valueToRead;
  bfs_Read("test_str", &valueToRead);

  EXPECT_EQ(valueToWrite.Type, valueToRead.Type);
  EXPECT_STREQ((const char*)valueToWrite.Value, (const char*)valueToRead.Value);
  EXPECT_STREQ("test_string", (const char*)valueToRead.Value);
  EXPECT_STREQ("test_string", (const char*)valueToRead.Padding) << "short strings are stored in padding";
  EXPECT_EQ(valueToWrite.SizeInBytes, valueToRead.SizeInBytes);
}

TEST(BifrostShared, StringInHeap) {
  std::string str = "test_string that is very very very very very very very very very very very long";

  bfs_Value valueToWrite{BFS_STRING, (uint64_t)str.data(), (uint32_t)str.size()};
  bfs_Write("test_str", &valueToWrite);

  bfs_Value valueToRead;
  bfs_Read("test_str", &valueToRead);

  EXPECT_EQ(valueToWrite.Type, valueToRead.Type);
  EXPECT_STREQ((const char*)valueToWrite.Value, (const char*)valueToRead.Value);
  EXPECT_EQ(valueToWrite.SizeInBytes, valueToRead.SizeInBytes);
  EXPECT_EQ(valueToWrite.SizeInBytes, str.size());
}

}  // namespace
