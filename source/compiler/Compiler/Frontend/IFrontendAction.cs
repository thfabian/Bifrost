using System;
using System.Collections.Generic;
using System.Text;

namespace Bifrost.Compiler.Frontend
{
    /// <summary>
    /// A fronted action produces a BIR
    /// </summary>
    public interface IFrontendAction
    {
        /// <summary>
        /// Build the Bifrost Intermediate Representation (BIR) from the input configuration
        /// </summary>
        /// <param name="config">Input configuration</param>
        /// <returns>BIR or null</returns>
        BIR.BIR Produce(Input.Configuration config);

        /// <summary>
        /// Update the BIR - this method is only called if one IFrontendAction.Produce returned a valid BIR.
        /// </summary>
        /// <param name="config">Input configuration</param>
        /// <param name="bir">Fully formed Bifrost Intermediate Representation</param>
        void Visit(Input.Configuration config, BIR.BIR bir);
    }
}
