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
using System.IO;

namespace Bifrost.Compiler.Core
{
    /// <summary>
    /// Location of a character in a file
    /// </summary>
    public class SourceLocation
    {
        /// <summary>
        /// Full path to the file
        /// </summary>
        public string File { get; }

        /// <summary>
        /// Row (i.e line) index, setting it to -1 indicates the entire file
        /// </summary>
        public int Row { get; }

        /// <summary>
        /// Column index in the Row, setting it to -1 indicates the entire row
        /// </summary>
        public int Column { get; }

        public SourceLocation(string file, int row = -1, int column = -1)
        {
            File = file;
            Row = row;
            Column = column;
        }

        public override string ToString()
        {
            var rowStr = Row == -1 ? "" : $"{Row}:";
            var colStr = Column == -1 ? "" : $"{Column}:";
            return $"{File}:{rowStr}{colStr}";
        }
    }
}
