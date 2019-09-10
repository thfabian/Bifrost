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
        public ILogger Logger => m_logger;

        public CompilerContext()
        {
        }

        /// <summary>
        /// Register a logger
        /// </summary>
        public void RegisterLogger(Logger.Base logger, string identifer = null)
        {
            m_logger.Loggers.Add(identifer ?? logger.ToString(), logger);
        }

        /// <summary>
        /// Try to register a logger, does nothing if the logger already exists
        /// </summary>
        public void TryRegisterLogger(Logger.Base logger, string identifer = null)
        {
            var ident = identifer ?? logger.ToString();
            if (!m_logger.Loggers.ContainsKey(ident))
            {
                m_logger.Loggers.Add(identifer ?? logger.ToString(), logger);
            }
        }

        /// <summary>
        /// Enable buffering of all log messages
        /// </summary>
        public void EnableLogBuffering()
        {
            m_logger.EnableBuffering(true);
        }

        /// <summary>
        /// Disable the buffering of log messages and flush all the buffered logs (if any)
        /// </summary>
        public void DisableLogBuffering()
        {
            m_logger.FlushBuffer();
            m_logger.EnableBuffering(false);
        }

        /// <summary>
        /// Run the given configuration
        /// </summary>
        public int Run(Configuration config)
        {
            return 0;
        }

        private Logger.Forward m_logger = new Forward();
    }
}
