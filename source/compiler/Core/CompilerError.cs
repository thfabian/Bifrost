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
    /// <summary>
    /// Compiler specific exception
    /// </summary>
    public class CompilerError : Exception
    {
        /// <summary>
        /// Source location the error occurred
        /// </summary>
        public SourceRange Range;

        public CompilerError(string message) : base(message) { }

        public CompilerError(string message, SourceLocation location) : base(message) { Range = new SourceRange(location, null); }

        public CompilerError(string message, SourceRange sourceRange) : base(message) { Range = sourceRange; }

    }
}
