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
using System.Text;
using System.Threading.Tasks;

namespace Bifrost.Compiler.Input
{
    public class Configuration
    {
        /// <summary>
        /// Clang specific options
        /// </summary>
        public class ClangT
        {
            /// <summary>
            /// Extra arguments passed to Clang
            /// </summary>
            public string Arguments = "";

        };
        public ClangT Clang = new ClangT();
    }
}
