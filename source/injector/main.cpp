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
#include "injector/process.h"
#include "injector/injector.h"
#include "injector/util.h"
#include "bifrost_core/logging.h"
#include <iostream>

using namespace injector;

namespace {

void PrintVersion() { std::cout << INJECTOR_VERSION_STRING << " (Bifrost " << bifrost_GetVersion() << ")" << std::endl; }

}  // namespace

int main(int argc, const char* argv[]) {
  // Extract the program name
  auto program = argc > 0 ? std::filesystem::path(argv[0]).filename().string() : "injector";
  Error::Get().SetProgram(program);

  // Setup logging
  Logger::Get().AddSinks({{"stdout", Logger::MakeStdoutSink()}, {"file", Logger::MakeFileSink("injector.log.txt")}, {"mscv", Logger::MakeMsvcSink()}});
  bifrost::Logging::Get().SetCallback(LogCallback);

  auto terminal = GetTerminalInfo();

  // Parse args
  args::ArgumentParser parser("Bifrost Injector (" INJECTOR_VERSION_STRING ")");
  parser.helpParams.width = terminal.Columns;
  parser.Prog(argc > 0 ? std::filesystem::path(argv[0]).filename().string() : "injector");

  args::HelpFlag help(parser, "help", "Display this help menu and exit.", {'h', "help"});
  args::Flag version(parser, "version", "Display version information and exit.", {'v', "version"});
  args::ValueFlag<std::string> exe(parser, "<path>", "Launch executable <path>.", {"exe"});
  args::ValueFlagList<std::string> exeArgs(parser, "<arg>", "Arguments passed to the launched executable.", {"exe-arg"});

  try {
    parser.ParseCLI(argc, argv);
  } catch (const args::Completion& e) {
    std::cout << e.what();
    return 0;
  } catch (const args::Help&) {
    std::cout << parser << std::endl;
    return 0;
  } catch (const args::ParseError& e) {
    Error::Get().Critical(e.what());
  }

  // Print version and exit
  if (version) {
    PrintVersion();
    return 0;
  }

  // Inject the bifrost dll
  try {
    std::unique_ptr<Process> process = nullptr;

    // Launch or query the process
    if (exe) {
      process = Process::Launch(std::filesystem::path(exe.Get()), exeArgs.Get());
    }

    Injector inject(std::move(process));

  } catch (std::runtime_error& e) {
    Error::Get().Critical(e.what());
  } catch (std::exception& e) {
    Error::Get().Critical("unexpected error: %s", e.what());
  }

  return 0;
}