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

/** Bifrost Core **/
#include "bifrost/core/common.h"
#include "bifrost/core/type.h"
#include "bifrost/core/json.h"

/** Bifrost Injector **/
#include "bifrost/api/injector.h"

/** args **/
#include <args.hxx>

/** spdlog **/
#define SPDLOG_NO_THREAD_ID
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/msvc_sink.h>

namespace injector {

using namespace bifrost;

}
