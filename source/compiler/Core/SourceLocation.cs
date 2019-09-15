using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace Bifrost.Compiler.Core
{
    /// <summary>
    /// Location of a character in a file
    /// </summary>
    public sealed class SourceLocation
    {
        public SourceLocation(string file, int row = -1, int column = -1)
        {
            File = file;
            Row = row;
            Column = column;
        }

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

        public override string ToString()
        {
            var rowStr = Row == -1 ? "" : $"{Row}:";
            var colStr = Column == -1 ? "" : $"{Column}:";
            return $"{File}:{rowStr}{colStr}";
        }
    }
}
