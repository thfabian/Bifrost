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

#pragma once

#include "bifrost/core/common.h"
#include "bifrost/core/mutex.h"
#include "bifrost/core/sm_object.h"

namespace bifrost {

class SMLogStash : public SMObject {
 public:
  void Destruct(SharedMemory* mem);

 private:
  SpinMutex m_mutex;
};

}  // namespace bifrost