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
    /// Location of a character in a file
    /// </summary>
    public class SourceRange
    {
        /// <summary>
        /// Get start location
        /// </summary>
        public SourceLocation Start { get; }

        /// <summary>
        /// Get end location
        /// </summary>
        public SourceLocation End { get; }

        public SourceRange(SourceLocation start, SourceLocation end = null)
        {
            Start = start;
            End = end;
        }
    }
}
