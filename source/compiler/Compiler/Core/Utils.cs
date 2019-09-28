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
