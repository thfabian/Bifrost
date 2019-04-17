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
#include "bifrost/core/new.h"

namespace {

using namespace bifrost;

class NewTest : public TestBase<true> {};
class Foo {
 public:
  int Integer = 1;
  float Float = 2.0f;

  Foo() = default;

  Foo(int i, float f) {
    Integer = i;
    Float = f;
  }
};

TEST_F(NewTest, Object) {
  Ptr<Foo> ptr;
  ASSERT_NO_THROW((ptr = New<Foo>(GetContext())));

  auto ptrV = Resolve(ptr);
  ASSERT_EQ(1, ptrV->Integer);
  ASSERT_EQ(2.0f, ptrV->Float);

  ASSERT_NO_THROW(Delete(GetContext(), ptr));
}

TEST_F(NewTest, ObjectConstructor) {
  Ptr<Foo> ptr;
  ASSERT_NO_THROW((ptr = New<Foo>(GetContext(), 2, 3.0f)));

  auto ptrV = Resolve(ptr);
  ASSERT_EQ(2, ptrV->Integer);
  ASSERT_EQ(3.0f, ptrV->Float);

  ASSERT_NO_THROW(Delete(GetContext(), ptr));
}

TEST_F(NewTest, Array) {
  Ptr<Foo> ptr;
  ASSERT_NO_THROW((ptr = NewArray<Foo>(GetContext(), 5)));

  auto ptrV = Resolve(ptr);
  ASSERT_EQ(1, ptrV[0].Integer);
  ASSERT_EQ(2.0f, ptrV[0].Float);

  ASSERT_EQ(1, ptrV[4].Integer);
  ASSERT_EQ(2.0f, ptrV[4].Float);

  ASSERT_NO_THROW(DeleteArray(GetContext(), ptr, 5));
}

}  // namespace