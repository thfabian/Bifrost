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
#include "bifrost/core/ilogger.h"
#include <iostream>

using namespace bifrost;
using namespace injector;

#define INJECTOR_LOG_FILE "injector.log.txt"
#define INJECTOR_CHECK_PTR(expr)                                  \
  if ((expr) == nullptr) {                                        \
    throw std::runtime_error(bfi_ContextGetLastError(ctx.get())); \
  }
#define INJECTOR_CHECK(expr)                                      \
  if ((expr) != BFI_OK) {                                         \
    throw std::runtime_error(bfi_ContextGetLastError(ctx.get())); \
  }

int main(int argc, const char* argv[]) {
  auto program = argc > 0 ? std::filesystem::path(argv[0]).filename().string() : "injector";

  try {
    // Extract the program name
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
    parser.Epilog(StringFormat("\nEXAMPLES:\n  %s --exe=foo.exe --plugin=bar.dll\n  %s --pid=19252 --plugin=\"bar.dll:--foo\" --plugin=foo.dll\n",
                               program.c_str(), program.c_str()));

    args::Group generalGroup(parser, "GENERAL:");
    args::HelpFlag help(generalGroup, "help", "Display this help menu and exit.", {'h', "help"}, args::Options::HiddenFromUsage);
    args::Flag version(generalGroup, "version", "Display version information and exit.", {'v', "version"}, args::Options::HiddenFromUsage);
    args::Flag quiet(generalGroup, "quiet", "Disable verbose logging.", {'q', "quiet"}, args::Options::HiddenFromUsage);
    args::ValueFlag<std::string> logFile(generalGroup, "file", "Log to <file> (default: " INJECTOR_LOG_FILE ")", {"log-file"}, INJECTOR_LOG_FILE,
                                         args::Options::HiddenFromUsage);

    args::Group exeGroup(parser, "EXECUTABLE:");
    args::ValueFlag<std::string> exe(exeGroup, "exe", "Launch executable <exe> given by the full path.", {"exe"});
    args::ValueFlag<u32> exeTimeout(exeGroup, "t", "Time out the executable after <t> seconds (default: infinite).", {"exe-timeout"},
                                    args::Options::HiddenFromUsage);
    args::ValueFlag<std::string> arg(exeGroup, "arg", "Pass arguments <arg> to the executable <exe>.", {"arg"});
    args::ValueFlag<i32> pid(exeGroup, "pid", "Connect to the process given by <pid>.", {"pid"});
    args::ValueFlag<std::string> name(exeGroup, "name", "Connect to the process <name>.", {"name"});

    args::Group pluginGroup(parser, "PLUGINS:");
    args::ValueFlagList<std::string> plugins(
        pluginGroup, "dll:arg",
        "Load plugin given by <dll> and optionally pass the arguments <arg> to it (separated by ':'). Example: --plugin=\"foo.dll:--foo --bar\". If the plugin "
        "is located in a different path, <dll> has to be a full path. This option can be repeated - the plugins are loaded in the provided order, meaning from "
        "left to right.",
        {"plugin"});

    args::Group injectorGroup(parser, "INJECTOR:");
    args::ValueFlag<u32> injectorTimeout(
        injectorGroup, "t",
        StringFormat("Time out the injection process after <t> seconds (default: %u).", BIFROST_INJECTOR_DEFAULT_InjectorArguments_InjectorTimeoutInS),
        {"injector-timeout"}, args::Options::HiddenFromUsage);
    args::ValueFlag<u32> sharedMemorySize(
        injectorGroup, "n",
        StringFormat("Set the shared memory size in bytes to <n> (default: %u).", BIFROST_INJECTOR_DEFAULT_InjectorArguments_SharedMemorySizeInBytes),
        {"shared-memory-size"}, args::Options::HiddenFromUsage);

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
    std::shared_ptr<bfi_Context> ctx(bfi_ContextInit(), [](bfi_Context* c) { bfi_ContextFree(c); });
    if (ctx == nullptr) throw std::runtime_error("Failed to initialize bifrost injector context");
    INJECTOR_CHECK(bfi_ContextSetLoggingCallback(ctx.get(), LogCallback));

    // Initialize injector arguments
    bfi_InjectorArguments* p = nullptr;
    INJECTOR_CHECK_PTR((p = bfi_InjectorArgumentsInit(ctx.get())));
    std::shared_ptr<bfi_InjectorArguments> args(p, [&](bfi_InjectorArguments* p) { INJECTOR_CHECK(bfi_InjectorArgumentsFree(ctx.get(), p)); });

    std::vector<std::unique_ptr<char>> strMem;
    std::vector<std::unique_ptr<wchar_t>> wstrMem;
    auto copyString = [&strMem](const std::string& str) -> const char* { return strMem.emplace_back(StringCopy(str)).get(); };
    auto copyWString = [&wstrMem](const std::wstring& str) -> const wchar_t* { return wstrMem.emplace_back(StringCopy(str)).get(); };

    if (exe) {
      args->Mode = BFI_LAUNCH;
      args->Executable = copyWString(StringToWString(exe.Get()));
      if (arg) args->Arguments = copyString(arg.Get());
    }
    if (pid) {
      args->Mode = BFI_CONNECT_VIA_PID;
      args->Pid = pid;
    }
    if (name) {
      args->Mode = BFI_CONNECT_VIA_NAME;
      args->Name = copyWString(StringToWString(name.Get()));
    }

    if (injectorTimeout) args->InjectorTimeoutInS = injectorTimeout;
    if (sharedMemorySize) args->SharedMemorySizeInBytes = sharedMemorySize;

    // Construct the plugins
    std::vector<bfi_Plugin> bfiPlugins;
    for (const auto& p : plugins) {
      auto idx = p.find_first_of(':');
      bfi_Plugin plugin = {0};
      if (idx != -1) {
        plugin.Path = copyWString(StringToWString(p.substr(0, idx)));
        plugin.Arguments = copyString(p.substr(idx + 1));
      } else {
        plugin.Path = copyWString(StringToWString(p));
        plugin.Arguments = NULL;
      }
      bfiPlugins.emplace_back(plugin);
    }
    args->Plugins = bfiPlugins.data();
    args->NumPlugins = (u32)bfiPlugins.size();

    // Launch/connect to the executable and inject the plugins
    bfi_Process* bfiProcess = nullptr;
    INJECTOR_CHECK(bfi_ProcessInject(ctx.get(), args.get(), &bfiProcess));
    std::shared_ptr<bfi_Process> process(bfiProcess, [&](bfi_Process* p) { INJECTOR_CHECK(bfi_ProcessFree(ctx.get(), p)); });

    // Wait for the process to complete
    int32_t exitCode = 0;
    // INJECTOR_CHECK(bfi_ProcessWait(ctx.get(), process.get(), exeTimeout ? exeTimeout.Get() : 0, &exitCode));

    LogCallback((u32)ILogger::LogLevel::Debug, program.c_str(), StringFormat("Setting exit code to process exit code: %i", exitCode).c_str());
    
    
    return exitCode;

  } catch (std::runtime_error& e) {
    LogCallback((u32)ILogger::LogLevel::Error, program.c_str(), e.what());
    Error::Get().Critical(e.what());
  } catch (std::exception& e) {
    LogCallback((u32)ILogger::LogLevel::Error, program.c_str(), StringFormat("Unexpected error: %s", e.what()).c_str());
    Error::Get().Critical("Unexpected error: %s", e.what());
  }
  return 0;
}