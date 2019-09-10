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

using CommandLine;
using CommandLine.Text;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bifrost.Compiler.Core;

namespace Bifrost.Compiler.Input
{
    /// <summary>
    /// Build the input configuration which configures the compiler passes
    /// </summary>
    public class ConfigurationBuilder : CompilerObject
    {
        public ConfigurationBuilder(CompilerContext ctx) : base(ctx) { }

        /// <summary>
        /// Build a configuration given the input files <paramref name="files"/> and the parsed command-line <paramref name="cmd"/>
        /// </summary>
        public Configuration Build(CommandLine cmd, string file)
        {
            return new Configuration();
        }
    }
}
