using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bifrost.Compiler.Logger;

namespace Bifrost.Compiler.Core
{
    /// <summary>
    /// Bifrost object
    /// </summary>
    public class CompilerObject
    {
        /// <summary>
        /// Underlying context
        /// </summary>
        public CompilerContext Context;

        /// <summary>
        /// Logger of the context
        /// </summary>
        public ILogger Logger => Context.Logger;

        public CompilerObject(CompilerContext ctx)
        {
            Context = ctx;
        }
    }
}
