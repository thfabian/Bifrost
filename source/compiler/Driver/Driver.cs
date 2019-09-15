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

using System;
using System.Collections.Generic;
using System.Linq;

namespace Bifrost.Compiler.Driver
{
    /// <summary>
    /// Invokes the compiler
    /// </summary>
    public static class Driver
    {
        public static int Run(IEnumerable<string> args)
        {
            var ctx = new Core.CompilerContext();

            // Parse command-line
            var cmd = ParseArguments(ctx, args);
            if (cmd == null)
            {
                return 1;
            }
            if (cmd.Stop)
            {
                return 0;
            }

            // Run the compiler 
            return RunCompiler(ctx, cmd);
        }

        /// <summary>
        /// Parse command-line options
        /// </summary>
        private static Input.CommandLine ParseArguments(Core.CompilerContext ctx, IEnumerable<string> args)
        {
            try
            {
                return Input.CommandLine.ParseArguments(ctx, args);
            }
            catch (Exception e)
            {
                ctx.Diagnostics.Error(e);
            }
            return null;
        }

        /// <summary>
        /// Run the compiler
        /// </summary>
        private static int RunCompiler(Core.CompilerContext ctx, Input.CommandLine cmd)
        {
            try
            {
                // 1) Build the configuration
                var configBuilder = new Input.ConfigurationBuilder(ctx);
                var config = configBuilder.Build(cmd, "");

                // 2) Run the configuration
                return ctx.Run(config);
            }
            catch (Core.CompilerError e)
            {
                ctx.Diagnostics.Error(e);
                return 1;
            }
            catch (Exception e)
            {
                ctx.Diagnostics.Error(e);
                return 1;
            }
        }
    }
}
