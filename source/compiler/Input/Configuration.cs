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

namespace Bifrost.Compiler.Input
{
    public class Configuration
    {
        /// <summary>
        /// General options
        /// </summary>
        public class GeneralT
        {
            /// <summary>
            /// Use verbose output
            /// </summary>
            public bool Logging = false;

            /// <summary>
            /// File to log to
            /// </summary>
            public string LogFile = "log.{0}.txt";

        };
        public GeneralT General = new GeneralT();
    }
}
