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
#include "injector/options.h"
#include "injector/util.h"
#include "bifrost/core/util.h"
#include "bifrost/core/ilogger.h"
#include "bifrost/core/json.h"
#include "bifrost/core/context.h"
#include "bifrost/core/module_loader.h"
#include <iostream>

using namespace bifrost;
using namespace injector;

#define INJECTOR_LOG_FILE "log.injector.txt"
#define INJECTOR_SHORT_PREFIX "-"
#define INJECTOR_LONG_PREFIX "--"

#define INJECTOR_CHECK_PTR(expr)                                  \
  if ((expr) == nullptr) {                                        \
    throw std::runtime_error(bfi_ContextGetLastError(ctx.get())); \
  }
#define INJECTOR_CHECK(expr)                                      \
  if ((expr) != BFP_OK) {                                         \
    throw std::runtime_error(bfi_ContextGetLastError(ctx.get())); \
  }

namespace {

/// Keep track of string memory
struct MemoryPool {
  std::vector<std::unique_ptr<char>> StrMem;
  std::vector<std::unique_ptr<wchar_t>> WstrMem;

  const char* CopyString(const std::string& str) { return StrMem.emplace_back(StringCopy(str)).get(); }
  const wchar_t* CopyString(const std::wstring& str) { return WstrMem.emplace_back(StringCopy(str)).get(); };
};

std::string GetMatcherOption(args::FlagBase* flag) { return flag->GetMatcher().GetLongOrAny().str(INJECTOR_SHORT_PREFIX, INJECTOR_LONG_PREFIX); }

/// Check of the flag has been set and throw on error
void Required(const args::Command& cmd, args::FlagBase* flag) {
  if (!flag->Matched()) {
    throw std::runtime_error(StringFormat("%s command: flag %s is required.", cmd.Name().c_str(), GetMatcherOption(flag).c_str()));
  }
}

/// Check if one and only one of the flags has been set
void RequiredAndExclusive(const args::Command& cmd, std::vector<args::FlagBase*>&& flags) {
  i32 numMatches = 0;
  for (const auto& flag : flags) numMatches += flag->Matched();

  if (numMatches == 1) return;
  if (numMatches == 0) {
    std::stringstream ss;
    auto sz = flags.size();
    for (i32 i = 0; i < flags.size(); ++i) ss << GetMatcherOption(flags[i]) << ((i == (sz - 1)) ? "" : (i == (sz - 2) ? " or " : ", "));
    throw std::runtime_error(StringFormat("%s command: flag %s is required.", cmd.Name().c_str(), ss.str().c_str()));

  } else {
    for (const auto& flag : flags) {
      if (flag->Matched()) {
        for (const auto& otherFlag : flags) {
          if (flag == otherFlag) continue;
          if (otherFlag->Matched())
            throw std::runtime_error(
                StringFormat("%s command: flags %s and %s are exclusive.", cmd.Name(), GetMatcherOption(flag).c_str(), GetMatcherOption(otherFlag).c_str()));
        }
      }
    }
  }
}

/// Print result to stdout as JSON
void PrintJsonOutput(bool success, const Json& output, const char* error) {
  Json jOutput;
  jOutput["Success"] = success;
  jOutput["Output"] = output;
  jOutput["Error"] = error;
  std::cout << jOutput.dump(2) << std::endl;
}

/// Print output to stdout
void PrintOutput(const Json& output) {
  for (const auto& [key, value] : output.items()) std::cout << StringFormat("%-15s: %s\n", key.c_str(), value.dump().c_str());
  std::cout.flush();
}

/// Thrown if everything went ok
class SuccessException : public std::exception {
 public:
  SuccessException(Json&& output) : Output(std::move(output)) {}
  Json Output;
};
void Success(Json output = {}) { throw SuccessException(std::move(output)); }

struct GeneralOptions {
  bool Json = false;
  bool Quiet = false;
  std::string LogFile = INJECTOR_LOG_FILE;
};

/// Options used to connect to an executable
struct ConnectOptions : public OptionCollection {
  enum OptionEnum { Pid, Name };

  ConnectOptions(args::Subparser& parser, const char* pidHelpMsg = "Connect to the executable given by <pid>.",
                 const char* nameHelpMsg = "Connect to the executable given by <name>.")
      : OptionCollection(parser) {
    AddOption(Pid, -1, new args::ValueFlag<i32>(parser, "pid", pidHelpMsg, {"pid"}));
    AddOption(Name, std::string{}, new args::ValueFlag<std::string>(parser, "name", nameHelpMsg, {"name"}));
  }
};

/// Options used for DLL injection
struct InjectorOptions : public OptionCollection {
  enum OptionEnum { Timeout, SharedMemorySize, SharedMemoryName, Debugger };

  InjectorOptions(args::Subparser& parser) : OptionCollection(parser) {
    u32 timeout = BIFROST_INJECTOR_DEFAULT_InjectorArguments_TimeoutInS;
    u32 size = BIFROST_INJECTOR_DEFAULT_InjectorArguments_SharedMemorySizeInBytes;

    AddOption(Timeout, timeout,
              new args::ValueFlag<u32>(parser, "t", StringFormat("Time out the injection process after <t> seconds (default: %u).", timeout),
                                       {"injector-timeout"}, args::Options::HiddenFromUsage));
    AddOption(SharedMemorySize, size,
              new args::ValueFlag<u32>(parser, "n", StringFormat("Set the shared memory size in bytes to <n> (default: %u).", size), {"shared-memory-size"},
                                       args::Options::HiddenFromUsage));
    AddOption(
        SharedMemoryName, std::string{},
        new args::ValueFlag<std::string>(parser, "name", "Set the shared memory name to <name>.", {"shared-memory-name"}, args::Options::HiddenFromUsage));
    AddOption(Debugger, std::string{},
              new args::ImplicitValueFlag<std::string>(parser, "solution",
                                                       "Attach a Visual Studio debugger. If multiple Visual Studio instances are running, connects to the one "
                                                       "with which has <solution> open - the <solution> argument is optional.",
                                                       {"debugger"}, args::Options::HiddenFromUsage));
  }
};

/// Options used to launching executables
struct ExecutableOptions : public OptionCollection {
  enum OptionEnum { Executable, Arguments };

  ExecutableOptions(args::Subparser& parser) : OptionCollection(parser) {
    AddOption(Executable, std::string{}, new args::ValueFlag<std::string>(parser, "exe", "Launch executable <exe> given by the full path.", {"exe"}));
    AddOption(Arguments, std::string{},
              new args::ValueFlag<std::string>(parser, "arg", "Pass arguments <arg> to the executable <exe>.", {"exe-arg"}, args::Options::HiddenFromUsage));
  }
};

}  // namespace

int main(int argc, const char* argv[]) {
  auto program = argc > 0 ? std::filesystem::path(argv[0]).filename().string() : "injector";

  GeneralOptions generalOptions;

  try {
    // Extract the program name
    Error::Get().SetProgram(program);

    // Parse args
    auto terminal = GetTerminalInfo();
    args::ArgumentParser parser("Bifrost Injector (" INJECTOR_VERSION_STRING ") - Inject and interact with plugins (dlls) of a remote process.");
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
    parser.helpParams.shortPrefix = INJECTOR_SHORT_PREFIX;
    parser.helpParams.longPrefix = INJECTOR_LONG_PREFIX;
    parser.Prog(argc > 0 ? std::filesystem::path(argv[0]).filename().string() : "injector");
    parser.Epilog(StringFormat("\nEXAMPLES:\n  %s --exe=foo.exe --plugin=bar.dll\n  %s --pid=19252 --plugin=\"bar.dll:bar\" --plugin-arg=\"bar:--foo\"\n",
                               program.c_str(), program.c_str()));

    args::Group generalGroup(parser, "GENERAL:");
    args::HelpFlag help(generalGroup, "help", "Display this help menu and exit.", {'h', "help"}, args::Options::HiddenFromUsage);
    args::Flag version(generalGroup, "version", "Display version information and exit.", {'v', "version"}, args::Options::HiddenFromUsage);
    args::Flag quiet(generalGroup, "quiet", "Disable verbose logging.", {'q', "quiet"}, args::Options::HiddenFromUsage);
    args::Flag json(generalGroup, "json", "Print output JSON formatted to stdout.", {"json"}, args::Options::HiddenFromUsage);
    args::ValueFlag<std::string> logFile(generalGroup, "file", "Log to <file> (default: " INJECTOR_LOG_FILE ")", {"log-file"}, INJECTOR_LOG_FILE,
                                         args::Options::HiddenFromUsage);
    
    Context bfCtx;
    ModuleLoader bfLoader(&bfCtx);

    auto ParseCommand = [&](args::Subparser& p) {
      p.Parse();
      generalOptions.Json = json.Get();
      generalOptions.Quiet = quiet.Get();
      generalOptions.LogFile = logFile.Get();

      Logger::Get().AddSinks({{"file", Logger::MakeFileSink(generalOptions.LogFile)}, {"mscv", Logger::MakeMsvcSink()}});
      if (!generalOptions.Quiet) Logger::Get().AddSink("stderr", Logger::MakeStderrSink());
      bfCtx.SetLogger(&Logger::Get());
      bfCtx.Logger().SetModule(program.c_str());
    };

    auto Warn = [&](std::string msg) {
      LogCallback((u32)ILogger::LogLevel::Warn, program.c_str(), msg.c_str());
      Error::Get().Warning(msg.c_str());
    };

    MemoryPool mem;
    args::Group commandGroup(parser, "COMMANDS:");

    // Load command
    args::Command loadCommand(
        commandGroup, "load", "Launch <exe> or connect to the executable <pid> or <name> and load the plugin(s).", [&](args::Subparser& launchParser) {
          auto connectOptions = ConnectOptions(launchParser);
          auto exeOptions = ExecutableOptions(launchParser);

          args::ValueFlag<u32> exeTimeout(launchParser, "t", "Wait for <t> seconds or until the executable returns (default: infinite).", {"timeout"},
                                          args::Options::HiddenFromUsage);
          args::Flag noWait(launchParser, "t", "Do not wait for the executable to return.", {"no-wait"}, args::Options::HiddenFromUsage);

          args::ValueFlagList<std::string> plugins(
              launchParser, "dll:name",
              "Load plugin given by <dll> and optionally specify a custom <name> of the plugin (separated by ':'). If <name> is not provided, the filename "
              "(without extension) of <dll> is used. If the plugin is located in a different path, <dll> has to be the full path to the dll. This option can "
              "be repeated - the plugins are loaded in the provided order, meaning from left to right.",
              {"plugin"});
          args::ValueFlagList<std::string> pluginArgs(launchParser, "name:args", "Pass <args> to the plugin <name>. This option can be repeated.",
                                                      {"plugin-arg"}, {}, args::Options::HiddenFromUsage);

          auto injectorOption = InjectorOptions(launchParser);
          ParseCommand(launchParser);

          RequiredAndExclusive(launchParser.GetCommand(), {exeOptions.GetFlag(ExecutableOptions::Executable), connectOptions.GetFlag(ConnectOptions::Pid),
                                                           connectOptions.GetFlag(ConnectOptions::Name)});
          Required(launchParser.GetCommand(), &plugins);

          // Initialize Bifrost Injector
          std::shared_ptr<bfi_Context> ctx(bfi_ContextInit(), [](bfi_Context* c) { bfi_ContextFree(c); });
          if (ctx == nullptr) throw std::runtime_error("Failed to initialize bifrost injector context");
          INJECTOR_CHECK(bfi_ContextSetLoggingCallback(ctx.get(), LogCallback));

          // Set up executable arguments (launch or connect)
          bfi_ExecutableArguments exeArguments;
          ZeroMemory(&exeArguments, sizeof(exeArguments));

          if (exeOptions.GetFlag(ExecutableOptions::Executable)->Matched()) {
            exeArguments.Mode = BFI_LAUNCH;
            exeArguments.ExecutablePath = mem.CopyString(StringToWString(exeOptions.GetValue<std::string>(ExecutableOptions::Executable)));
            if (*exeOptions.GetFlag(ExecutableOptions::Arguments))
              exeArguments.ExecutableArguments = mem.CopyString(exeOptions.GetValue<std::string>(ExecutableOptions::Arguments));
          } else if (connectOptions.GetFlag(ConnectOptions::Pid)->Matched()) {
            exeArguments.Mode = BFI_CONNECT_VIA_PID;
            exeArguments.Pid = connectOptions.GetValue<u32>(ConnectOptions::Pid);
          } else if (connectOptions.GetFlag(ConnectOptions::Name)->Matched()) {
            exeArguments.Mode = BFI_CONNECT_VIA_NAME;
            exeArguments.Name = mem.CopyString(StringToWString(connectOptions.GetValue<std::string>(ConnectOptions::Name)));
          }

          // Setup injector arguments
          bfi_InjectorArguments injectorArguments;
          ZeroMemory(&injectorArguments, sizeof(injectorArguments));

          injectorArguments.TimeoutInS = injectorOption.GetValue<u32>(InjectorOptions::Timeout);
          injectorArguments.SharedMemorySizeInBytes = injectorOption.GetValue<u32>(InjectorOptions::SharedMemorySize);

          auto sharedMemoryName = injectorOption.GetValue<std::string>(InjectorOptions::SharedMemoryName);
          injectorArguments.SharedMemoryName = sharedMemoryName.empty() ? NULL : mem.CopyString(sharedMemoryName);

          injectorArguments.Debugger = injectorOption.GetFlag(InjectorOptions::Debugger)->Matched() || ::IsDebuggerPresent();
          auto debugger = injectorOption.GetValue<std::string>(InjectorOptions::Debugger);
          injectorArguments.VSSolution = debugger.empty() ? NULL : mem.CopyString(StringToWString(debugger));

          // Setup plugin arguments
          std::unordered_map<std::string, std::string> bfiPluginArguments;
          for (const auto& p : pluginArgs) {
            auto idx = p.find_first_of(':');
            if (idx == -1) throw std::runtime_error(StringFormat("Invalid format of --plugin-arg=\"%s\": expected <name>:<args> pair, missing ':'", p.c_str()));

            auto name = p.substr(0, idx);
            auto args = p.substr(idx + 1);

            auto it = bfiPluginArguments.find(name);
            if (it != bfiPluginArguments.end()) {
              it->second += " " + args;
            } else {
              bfiPluginArguments.emplace(std::move(name), std::move(args));
            }
          }
          std::unordered_set<std::string> seenPlugins;

          // Construct the plugins
          std::vector<bfi_Plugin> bfiPlugins;
          for (const auto& p : plugins) {
            std::string name, args;

            std::wstring pathStr;
            std::filesystem::path path;

            // Extract path and name
            auto idx = p.find_first_of(':');
            if (idx != -1) {
              pathStr = StringToWString(p.substr(0, idx));
              name = p.substr(idx + 1);
            } else {
              pathStr = StringToWString(p);
              name = std::filesystem::path(path).stem().string();
            }

            // 1) Use path relative to the current working directory
            // 2) Use path is relative to the executable
            // 3) Fall-back and use path which has been provided
            path = std::filesystem::absolute(pathStr);
            if (!std::filesystem::exists(path)) {
              path = std::filesystem::path(bfLoader.GetCurrentModulePath()).parent_path() / pathStr;
              if (!std::filesystem::exists(path)) {
                path = pathStr;
              }
            }

            // Associate arguments
            auto it = bfiPluginArguments.find(name);
            if (it != bfiPluginArguments.end()) {
              args = it->second;
              seenPlugins.emplace(name);
            }

            bfi_Plugin plugin;
            plugin.Name = name.empty() ? NULL : mem.CopyString(name);
            plugin.Path = path.empty() ? NULL : mem.CopyString(path.native());
            plugin.Arguments = args.empty() ? NULL : mem.CopyString(args);

            bfiPlugins.emplace_back(plugin);
          }

          // Warn about arguments which were not used
          for (const auto& p : bfiPluginArguments) {
            if (seenPlugins.count(p.first) == 0) Warn(StringFormat("Arguments of plugin \"%s\" were not used", p.first.c_str()));
          }

          // Launch/connect to the executable and inject the plugins
          bfi_PluginLoadArguments pluginLoadArguments;
          ZeroMemory(&pluginLoadArguments, sizeof(pluginLoadArguments));

          pluginLoadArguments.Executable = &exeArguments;
          pluginLoadArguments.InjectorArguments = &injectorArguments;
          pluginLoadArguments.Plugins = bfiPlugins.data();
          pluginLoadArguments.NumPlugins = (u32)bfiPlugins.size();

          bfi_Process* bfiProcess = nullptr;
          bfi_PluginLoadResult* bfiPluginLoadResult = nullptr;

          INJECTOR_CHECK(bfi_PluginLoad(ctx.get(), &pluginLoadArguments, &bfiProcess, &bfiPluginLoadResult))

          std::shared_ptr<bfi_Process> process(bfiProcess, [&](bfi_Process* p) { INJECTOR_CHECK(bfi_ProcessFree(ctx.get(), p)); });
          std::shared_ptr<bfi_PluginLoadResult> pluginLoadResult(bfiPluginLoadResult,
                                                                 [&](bfi_PluginLoadResult* p) { INJECTOR_CHECK(bfi_PluginLoadResultFree(ctx.get(), p)); });

          int32_t exitCode = STILL_ACTIVE;
          if (!noWait) {
            INJECTOR_CHECK(bfi_ProcessWait(ctx.get(), process.get(), exeTimeout ? exeTimeout.Get() : 0, &exitCode));
            LogCallback((u32)ILogger::LogLevel::Debug, program.c_str(), StringFormat("Exit code of remote process: %i", exitCode).c_str());
          }

          Success({
              {"SharedMemoryName", bfiPluginLoadResult->SharedMemoryName},
              {"SharedMemorySize", bfiPluginLoadResult->SharedMemorySize},
              {"RemoteProcessPid", bfiPluginLoadResult->RemoteProcessPid},
              {"RemoteProcessExitCode", exitCode},
          });
        });

    // Unload command

    // Launch command

    // Message command

    // Wait command

    // Kill command
    args::Command killCommand(commandGroup, "kill", "Kill the executable(s) given by <pid> or <name>.", [&](args::Subparser& launchParser) {
      auto connectOptions = ConnectOptions(launchParser, "Kill the executable given by <pid>.", "Kill all executables given by <name>.");
      ParseCommand(launchParser);

      RequiredAndExclusive(launchParser.GetCommand(), {connectOptions.GetFlag(ConnectOptions::Pid), connectOptions.GetFlag(ConnectOptions::Name)});

      std::shared_ptr<bfi_Context> ctx(bfi_ContextInit(), [](bfi_Context* c) { bfi_ContextFree(c); });
      INJECTOR_CHECK(bfi_ContextSetLoggingCallback(ctx.get(), LogCallback));
      if (*connectOptions.GetFlag(ConnectOptions::Name)) {
        INJECTOR_CHECK(bfi_ProcessKillByName(ctx.get(), StringToWString(connectOptions.GetValue<std::string>(ConnectOptions::Name)).c_str()));
      } else {
        INJECTOR_CHECK(bfi_ProcessKillByPid(ctx.get(), connectOptions.GetValue<i32>(ConnectOptions::Pid)));
      }
      Success();
    });

    // Parse command line
    try {
      parser.ParseCLI(argc, argv);
    } catch (const args::Completion& e) {
      std::cerr << e.what();
      return 0;
    } catch (const args::Help&) {
      std::cout << parser << std::endl;
      return 0;
    } catch (const args::Error& e) {
      if (version) {
        // Print version and exit
        std::cout << INJECTOR_VERSION_STRING << std::endl;
        return 0;
      } else {
        std::stringstream ss;
        ss << e.what();
        if (argc == 1) ss << "\n\n" << parser;
        Error::Get().Critical(ss.str().c_str());
      }
    }

  } catch (SuccessException& e) {
    if (generalOptions.Json) {
      PrintJsonOutput(true, e.Output, "");
    } else if(!generalOptions.Quiet) {
      PrintOutput(e.Output);
    }
    return 0;

  } catch (std::runtime_error& e) {
    LogCallback((u32)ILogger::LogLevel::Error, program.c_str(), e.what());

    if (generalOptions.Json) {
      PrintJsonOutput(false, {}, e.what());
      return 1;
    } else {
      Error::Get().Critical(e.what());
    }

  } catch (std::exception& e) {
    auto errMsg = StringFormat("Unexpected error: %s", e.what());
    LogCallback((u32)ILogger::LogLevel::Error, program.c_str(), errMsg.c_str());

    if (generalOptions.Json) {
      PrintJsonOutput(false, {}, errMsg.c_str());
      return 1;
    } else {
      Error::Get().Critical(errMsg.c_str());
    }
  }

  assert(0 && "unreachable");
  return 0;
}