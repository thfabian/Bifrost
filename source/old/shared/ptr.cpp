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

#include "bifrost/shared/common.h"
#include "bifrost/shared/ptr.h"
#include <iostream>

namespace bifrost::shared::internal {

std::ostream& StreamOffset(std::ostream& os, u64 offset) noexcept { return (os << offset); }

}  // namespace bifrost::shared::internal
