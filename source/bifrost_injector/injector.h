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

#include "bifrost_injector/common.h"

namespace bifrost::injector {

/// Inject bifrost into the provided executables and load the plugins
class Injector {
 public:
  /// Arguments to the injector
  class Arguments {
   public:
    enum Type {
      Launch,
      Connect,
    };

    virtual ~Arguments() = default;
    virtual Type GetType() const = 0;
  };

  class LaunchArguments : public Arguments {
   public:
    std::string Executable;  ///< Path to executable to launch
    std::string Arguments;   ///< Arguments passed to executable

    virtual ~LaunchArguments() = default;
    virtual Type GetType() const override { return Launch; }
  };

  class ConnectArguments : public Arguments {
   public:
    i32 Pid = -1;      ///< Process Identifier
    std::string Name;  ///< Name of the process

    virtual ~ConnectArguments() = default;
    virtual Type GetType() const override { return Connect; }
  };

  /// Initialize with args
  Injector(std::unique_ptr<Arguments> args);

  /// Inject into the executable and wait for the process to return (returns exit code of the launched/connected process)
  void InjectAndWait();

 private:
  std::unique_ptr<Arguments> m_args;
};

}  // namespace bifrost::injector