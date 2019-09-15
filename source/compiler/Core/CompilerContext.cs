﻿//   ____  _  __               _
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
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bifrost.Compiler.Input;
using Bifrost.Compiler.Logger;

namespace Bifrost.Compiler.Core
{
    /// <summary>
    /// Transient storage of a run 
    /// </summary>
    public class CompilerContext
    {
        /// <summary>
        /// Logger of the context
        /// </summary>
        public Logger.Forward Logger => m_logger;

        /// <summary>
        /// Diagnostic reporter
        /// </summary>
        public Diagnostics Diagnostics => m_diagnostics ?? new Diagnostics(this);
        private readonly Diagnostics m_diagnostics = null;

        public CompilerContext()
        {
        }

        /// <summary>
        /// Run the given configuration
        /// </summary>
        public int Run(Configuration config)
        {
            return 0;
        }

        private readonly Logger.Forward m_logger = new Forward();
    }
}
