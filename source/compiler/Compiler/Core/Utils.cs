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
using System.Text;

namespace Bifrost.Compiler.Core
{
    public class Utils : CompilerObject
    {
        public Utils(CompilerContext ctx) : base(ctx)
        {
        }

        /// <summary>
        /// Generate a unique identifier
        /// </summary>
        public string UUID()
        {
            return Guid.NewGuid().ToString();
        }
    }
}
