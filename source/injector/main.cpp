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

#include "injector/common.h"
#include "injector/version.h"
#include "injector/logger.h"
#include "injector/error.h"
#include "injector/util.h"
#include "bifrost/core/util.h"
#include <iostream>

using namespace bifrost;
using namespace injector;

#define BIFORST_INJECTOR_CHECK_PTR(expr)             \
  if ((expr) == nullptr) {                           \
    throw std::runtime_error(bfi_GetLastError(ctx)); \
  }
#define BIFORST_INJECTOR_CHECK(expr)                 \
  if ((expr) != BFI_OK) {                            \
    throw std::runtime_error(bfi_GetLastError(ctx)); \
  }

int main(int argc, const char* argv[]) {
  try {
    // Extract the program name
    auto program = argc > 0 ? std::filesystem::path(argv[0]).filename().string() : "injector";
    Error::Get().SetProgram(program);

    // Setup logging
    Logger::Get().AddSinks({{"stderr", Logger::MakeStderrSink()}, {"file", Logger::MakeFileSink("injector.log.txt")}, {"mscv", Logger::MakeMsvcSink()}});

    // Initialize Bifrost Injector
    bfi_Context* ctx = bfi_Init();
    if (ctx == nullptr) {
      throw std::runtime_error("Failed to initialize bifrost injector");
    }
    BIFORST_INJECTOR_CHECK(bfi_SetCallback(ctx, LogCallback));

    // Parse args
    auto terminal = GetTerminalInfo();
    args::ArgumentParser parser("Bifrost Injector (" INJECTOR_VERSION_STRING ")");
    parser.helpParams.width = terminal.Columns - 1;
    parser.helpParams.addDefault = false;
    parser.helpParams.eachgroupindent = 1;
    parser.helpParams.showCommandChildren = true;
    parser.helpParams.showCommandFullHelp = true;
    parser.helpParams.valueOpen = "<";
    parser.helpParams.valueClose = ">";
    parser.helpParams.proglineShowFlags = true;
    parser.Prog(argc > 0 ? std::filesystem::path(argv[0]).filename().string() : "injector");

    args::HelpFlag help(parser, "help", "Display this help menu and exit.", {'h', "help"}, args::Options::HiddenFromUsage);
    args::Flag version(parser, "version", "Display version information and exit.", {'v', "version"}, args::Options::HiddenFromUsage);

    args::ValueFlag<std::string> plugins(
        parser, "dll:arg",
        "Load plugin given by <dll> and optionally pass the arguments <arg> to it (separated by ':'). For example --plugin=\"foo.dll:--foo --bar\". This "
        "option can be repeated - the plugins are loaded in the provided order, meaning from left to right.",
        {"plugin"});

    args::ValueFlag<std::string> exe(parser, "exe", "Launch executable <exe>.", {"exe"});
    args::ValueFlag<std::string> arg(parser, "arg", "Pass arguments <arg> to the executable <exe>.", {"arg"});
    args::ValueFlag<i32> pid(parser, "pid", "Connect to the process given by <pid>.", {"pid"});
    args::ValueFlag<std::string> name(parser, "name", "Connect to the process <name>.", {"name"});

    try {
      parser.ParseCLI(argc, argv);
    } catch (const args::Completion& e) {
      std::cout << e.what();
      return 0;
    } catch (const args::Help&) {
      std::cout << parser << std::endl;
      return 0;
    } catch (const args::Error& e) {
      std::stringstream ss;
      ss << e.what();
      if (argc == 1) {
        ss << "\n\n" << parser;
      }
      Error::Get().Critical(ss.str().c_str());
    }

    // Print version and exit
    if (version) {
      std::cout << INJECTOR_VERSION_STRING << std::endl;
      return 0;
    }

    // Initialize injector arguments
    bfi_InjectorArguments* p = nullptr;
    BIFORST_INJECTOR_CHECK_PTR((p = bfi_InjectorArgumentsInit(ctx)));
    std::shared_ptr<bfi_InjectorArguments> injectorArgs(p, [&](bfi_InjectorArguments* p) { BIFORST_INJECTOR_CHECK(bfi_InjectorArgumentsFree(ctx, p)); });

  } catch (std::runtime_error& e) {
    Error::Get().Critical(e.what());
  } catch (std::exception& e) {
    Error::Get().Critical("Unexpected error: %s", e.what());
  }

  return 0;
}