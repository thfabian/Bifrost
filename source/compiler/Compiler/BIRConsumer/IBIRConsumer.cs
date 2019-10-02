using System;
using System.Collections.Generic;
using System.Text;

namespace Bifrost.Compiler.BIRConsumer
{
    /// <summary>
    /// Consume a BIR
    /// </summary>
    public interface IBIRConsumer
    {
        /// <summary>
        /// Consume the BIR <paramref name="bir"/> 
        /// </summary>
        /// <param name="config">Input configuration</param>
        /// <param name="bir">Bifrost Intermediate Representation (BIR)</param>
        /// <returns>True on success, False otherwise</returns>
        bool Consume(Input.Configuration config, BIR.BIR bir);
    }
}
