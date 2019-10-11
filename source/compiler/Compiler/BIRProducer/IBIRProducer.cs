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

namespace Bifrost.Compiler.BIRProducer
{
    /// <summary>
    /// Prouce a Bifrost Intermediate Representation (BIR)
    /// </summary>
    public interface IBIRProducer
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
