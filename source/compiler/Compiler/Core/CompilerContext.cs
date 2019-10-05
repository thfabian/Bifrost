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
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bifrost.Compiler.Input;

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
        public Logger.Forward Logger { get; } = new Logger.Forward();

        /// <summary>
        /// Diagnostic reporter
        /// </summary>
        public Diagnostics Diagnostics => m_diagnostics ?? (m_diagnostics = new Diagnostics(this));
        private Diagnostics m_diagnostics = null;

        /// <summary>
        /// I/O related functions
        /// </summary>
        public IO IO => m_io ?? (m_io = new IO(this));
        private IO m_io = null;

        /// <summary>
        /// I/O related functions
        /// </summary>
        public Utils Utils => m_utils ?? (m_utils = new Utils(this));
        private Utils m_utils = null;

        /// <summary>
        /// Internal Profiler
        /// </summary>
        public Profiler Profiler => m_profiler ?? (m_profiler = new Profiler(this));
        private Profiler m_profiler = null;

        public CompilerContext() { }

        /// <summary>
        /// Run the given configuration
        /// </summary>
        public bool Run(Configuration config)
        {
            return new Compiler(this).Run(config);
        }
    }
}
