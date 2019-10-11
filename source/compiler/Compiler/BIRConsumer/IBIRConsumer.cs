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
