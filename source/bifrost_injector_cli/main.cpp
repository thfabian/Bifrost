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

#include "bifrost_injector/common.h"
#include "bifrost_injector/version.h"
#include "bifrost_injector/logger.h"
#include "bifrost_injector/error.h"
#include "bifrost_injector/util.h"
#include "bifrost_injector/injector.h"
#include "bifrost_core/logging.h"
#include "bifrost_core/util.h"
#include <iostream>
#include <args.hxx>

using namespace bifrost;
using namespace bifrost::injector;

int main(int argc, const char* argv[]) {
  try {
    // Extract the program name
    auto program = argc > 0 ? std::filesystem::path(argv[0]).filename().string() : "injector";
    Error::Get().SetProgram(program);

    // Setup logging
    Logger::Get().AddSinks({{"stderr", Logger::MakeStderrSink()}, {"file", Logger::MakeFileSink("injector.log.txt")}, {"mscv", Logger::MakeMsvcSink()}});
    bifrost::Logging::Get().SetCallback("injector", LogCallback);
    bifrost::Logging::Get().LogStateAsync(false);

    auto terminal = GetTerminalInfo();

    // Parse args
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

    std::unique_ptr<Injector::Arguments> injectArgs = nullptr;

    args::HelpFlag help(parser, "help", "Display this help menu and exit.", {'h', "help"}, args::Options::HiddenFromUsage);
    args::Flag version(parser, "version", "Display version information and exit.", {'v', "version"}, args::Options::HiddenFromUsage);

    args::ValueFlag<std::string> plugins(
        parser, "dll:arg",
        "Load plugin given by <dll> and optionally pass the arguments <arg> to it (separated by ':'). For example --plugin=\"foo.dll:--foo --bar\". This "
        "option can be repeated - the plugins are loaded in the provided order, meaning from left to right.",
        {"plugin"});

    // Launch command
    args::Command launchCommand(parser, "launch", "Launch the executable <exe> with <arg> and inject the plugins.", [&](args::Subparser& launchParser) {
      args::Positional<std::string> exe(launchParser, "exe", "Launch executable <exe>.");
      args::PositionalList<std::string> arg(launchParser, "arg", "Pass arguments <arg> to the executable <exe>.");
      launchParser.Parse();

      if (!exe) {
        throw std::runtime_error("launch command: <exe> argument is required");
      }

      auto args = std::make_unique<Injector::LaunchArguments>();
      args->Executable = exe;
      args->Arguments = arg;
      injectArgs = StaticUniquePointerCast<Injector::Arguments>(std::move(args));
    });

    // Connect command
    args::Command connectCommand(parser, "connect", "Connect to the provided executable and inject the plugins.", [&](args::Subparser& connectParser) {
      args::ValueFlag<i32> pid(connectParser, "pid", "Process identifier of the executable.", {"pid"});
      args::ValueFlag<std::string> name(connectParser, "name", "Name of the executable.", {"name"});
      connectParser.Parse();

      if (!pid || !name) {
        throw std::runtime_error("connect command: --pid or --name is required");
      }

      auto args = std::make_unique<Injector::ConnectArguments>();
      if (pid) args->Pid = pid;
      if (name) args->Name = name;
      injectArgs = StaticUniquePointerCast<Injector::Arguments>(std::move(args));
    });

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

    Injector injector(std::move(injectArgs));
    injector.InjectAndWait();

  } catch (std::runtime_error& e) {
    Error::Get().Critical(e.what());
  } catch (std::exception& e) {
    Error::Get().Critical("Unexpected error: %s", e.what());
  }

  return 0;
}