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
        public SourceLocation Location;

        public CompilerError(string message) : base(message) { }

        public CompilerError(string message, SourceLocation location) : base(message) { Location = location; }
    }
}
