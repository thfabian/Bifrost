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
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Bifrost.Compiler.Input
{
    /// <summary>
    /// Command line options which are exposed to the user
    /// </summary>
    public class CommandLine
    {
        [Option('f', "file", Required = true, HelpText = "Input YAML or JSON file containing the configuration.")]
        public string File { get; set; } = null;

        [Option('l', "logging", Required = false, HelpText = "Enable logging to console.")]
        public bool Logging { get; set; } = false;

        [Option("no-color", Required = false, HelpText = "Disabled colored output.")]
        public bool DisableColors { get; set; } = false;

        /// <summary>
        /// Update the <paramref name="config"/> with the parsed fields
        /// </summary>
        public void UpdateConfiguration(Configuration config)
        {
            config.General.Logging = Logging;
        }
    }
}
