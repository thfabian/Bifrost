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
#include "bifrost/core/sm_context.h"

namespace {

using namespace bifrost;

class SMContextTest : public TestBaseNoSharedMemory {};

TEST_F(SMContextTest, CreateAndMap) {
  auto mem1 = CreateSharedMemory();
  ASSERT_EQ(1, mem1->GetSMContext()->GetRefCount());

  auto mem2 = CreateSharedMemory();
  ASSERT_EQ(2, mem1->GetSMContext()->GetRefCount());
  ASSERT_EQ(2, mem2->GetSMContext()->GetRefCount());

  {
    auto mem3 = CreateSharedMemory();
    ASSERT_EQ(3, mem1->GetSMContext()->GetRefCount());
    ASSERT_EQ(3, mem2->GetSMContext()->GetRefCount());
    ASSERT_EQ(3, mem3->GetSMContext()->GetRefCount());
  }

  ASSERT_EQ(2, mem1->GetSMContext()->GetRefCount());
  ASSERT_EQ(2, mem2->GetSMContext()->GetRefCount());
}

}  // namespace