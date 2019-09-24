using System;
using System.Collections.Generic;
using System.Text;
using Bifrost.Compiler.BIR;
using Bifrost.Compiler.Input;
using Bifrost.Compiler.Core;

namespace Bifrost.Compiler.Frontend
{
    /// <summary>
    /// Clang based BIR producer
    /// </summary>
    public class Clang : CompilerObject, IFrontendAction
    {
        public Clang(CompilerContext ctx) : base(ctx) { }

        /// <inheritDoc />
        public BIR.BIR Produce(Configuration config)
        {
            throw new NotImplementedException();
        }

        /// <inheritDoc />
        public void Visit(Configuration config, BIR.BIR bir)
        {
        }
    }
}
