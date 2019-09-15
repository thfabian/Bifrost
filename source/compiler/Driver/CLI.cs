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

namespace Bifrost.Compiler.Driver
{
    /// <summary>
    /// Command Line Interface
    /// </summary>
    class CLI
    {
        /// <summary>
        /// Main entry point
        /// </summary>
        static int Main(string[] args)
        {
            return Driver.Run(args);
        }
    }
}
