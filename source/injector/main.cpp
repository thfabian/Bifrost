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

#define INJECTOR_LOG_FILE "injector.log.txt"
#define INJECTOR_CHECK_PTR(expr)                           \
  if ((expr) == nullptr) {                                 \
    throw std::runtime_error(bfi_GetLastError(ctx.get())); \
  }
#define INJECTOR_CHECK(expr)                               \
  if ((expr) != BFI_OK) {                                  \
    throw std::runtime_error(bfi_GetLastError(ctx.get())); \
  }

int main(int argc, const char* argv[]) {
  try {
    // Extract the program name
    auto program = argc > 0 ? std::filesystem::path(argv[0]).filename().string() : "injector";
    Error::Get().SetProgram(program);

    // Parse args
    auto terminal = GetTerminalInfo();
    args::ArgumentParser parser("Bifrost Injector (" INJECTOR_VERSION_STRING ") - Inject plugins (dlls) into a process.");
    parser.helpParams.width = terminal.Columns - 1;
    parser.helpParams.addDefault = false;
    parser.helpParams.eachgroupindent = 1;
    parser.helpParams.showCommandChildren = true;
    parser.helpParams.showCommandFullHelp = true;
    parser.helpParams.valueOpen = "<";
    parser.helpParams.valueClose = ">";
    parser.helpParams.proglineShowFlags = true;
    parser.helpParams.descriptionindent = 2;
    parser.helpParams.flagindent = 4;
    parser.Prog(argc > 0 ? std::filesystem::path(argv[0]).filename().string() : "injector");
    parser.Epilog(StringFormat("\nEXAMPLES:\n  %s --exe=foo.exe --plugin=bar.dll\n  %s --pid=19252 --plugin=\"bar.dll:--foo\" --plugin=foo.dll\n", program.c_str(), program.c_str()));

    args::Group generalGroup(parser, "GENERAL:");
    args::HelpFlag help(generalGroup, "help", "Display this help menu and exit.", {'h', "help"}, args::Options::HiddenFromUsage);
    args::Flag version(generalGroup, "version", "Display version information and exit.", {'v', "version"}, args::Options::HiddenFromUsage);
    args::Flag quiet(generalGroup, "quiet", "Disable verbose logging.", {'q', "quiet"}, args::Options::HiddenFromUsage);
    args::ValueFlag<std::string> logFile(generalGroup, "file", "Log to <file> (default: " INJECTOR_LOG_FILE ")", {"log-file"}, INJECTOR_LOG_FILE,
                                         args::Options::HiddenFromUsage);

    args::Group exeGroup(parser, "EXECUTABLE:");
    args::ValueFlag<std::string> exe(exeGroup, "exe", "Launch executable <exe> given by the full path.", {"exe"});
    args::ValueFlag<std::string> arg(exeGroup, "arg", "Pass arguments <arg> to the executable <exe>.", {"arg"});
    args::ValueFlag<i32> pid(exeGroup, "pid", "Connect to the process given by <pid>.", {"pid"});
    args::ValueFlag<std::string> name(exeGroup, "name", "Connect to the process <name>.", {"name"});

    args::Group pluginGroup(parser, "PLUGINS:");
    args::ValueFlag<std::string> plugins(
        pluginGroup, "dll:arg",
        "Load plugin given by <dll> and optionally pass the arguments <arg> to it (separated by ':'). For example --plugin=\"foo.dll:--foo --bar\". This "
        "option can be repeated - the plugins are loaded in the provided order, meaning from left to right.",
        {"plugin"});

    args::Group injectorGroup(parser, "INJECTOR:");
    args::ValueFlag<u32> timeout(injectorGroup, "t", "Time allocated for the injection process (in ms).", {"injector-timeout"}, args::Options::HiddenFromUsage);

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
      if (argc == 1) ss << "\n\n" << parser;
      Error::Get().Critical(ss.str().c_str());
    }

    // Print version and exit
    if (version) {
      std::cout << INJECTOR_VERSION_STRING << std::endl;
      return 0;
    }

    // Check options
    if (!exe && !pid && !name) {
      std::cout << parser << std::endl;
      throw std::runtime_error("No executable specified: argument --exe, --name or --pid is required");
    }
    if (exe && pid && name) throw std::runtime_error("Arguments --exe, --name and --pid are exclusive");
    if (exe && pid) throw std::runtime_error("Arguments --exe and --pid are exclusive");
    if (exe && name) throw std::runtime_error("Arguments --exe and --name are exclusive");
    if (name && pid) throw std::runtime_error("Arguments --pid and --name are exclusive");
    if (!plugins) Error::Get().Warning("No plugins specified - missing argument --plugin");

    // Setup logging
    Logger::Get().AddSinks({{"file", Logger::MakeFileSink(logFile.Get())}, {"mscv", Logger::MakeMsvcSink()}});
    if (!quiet) Logger::Get().AddSink("stderr", Logger::MakeStderrSink());

    // Initialize Bifrost Injector
    std::shared_ptr<bfi_Context> ctx(bfi_Init(), [](bfi_Context* c) { bfi_Free(c); });
    if (ctx == nullptr) throw std::runtime_error("Failed to initialize bifrost injector context");
    INJECTOR_CHECK(bfi_SetCallback(ctx.get(), LogCallback));

    // Initialize injector arguments
    bfi_InjectorArguments* p = nullptr;
    INJECTOR_CHECK_PTR((p = bfi_InjectorArgumentsInit(ctx.get())));
    std::shared_ptr<bfi_InjectorArguments> args(p, [&](bfi_InjectorArguments* p) { INJECTOR_CHECK(bfi_InjectorArgumentsFree(ctx.get(), p)); });

    std::vector<bfi_Plugin> bfiPlugins;

  } catch (std::runtime_error& e) {
    Error::Get().Critical(e.what());
  } catch (std::exception& e) {
    Error::Get().Critical("Unexpected error: %s", e.what());
  }

  return 0;
}