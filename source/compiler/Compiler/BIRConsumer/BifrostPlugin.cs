using System;
using System.Collections.Generic;
using System.Text;
using System.Linq;
using System.IO;
using Bifrost.Compiler.BIR;
using Bifrost.Compiler.Input;
using Bifrost.Compiler.Core;

namespace Bifrost.Compiler.BIRConsumer
{
    /// <summary>
    /// Generate a Bifrost Plugin
    /// </summary>
    public class BifrostPlugin : CompilerObject, IBIRConsumer
    {
        public BifrostPlugin(CompilerContext ctx) : base(ctx)
        {
        }

        /// <inheritDoc />
        public bool Consume(Configuration config, BIR.BIR bir)
        {
            return true;
        }
    }
}
