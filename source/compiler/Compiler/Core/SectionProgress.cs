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
using System.Text;
using System.Diagnostics;
using System.IO;

namespace Bifrost.Compiler.Core
{
    /// <summary>
    /// Log progress update
    /// </summary>
    public class SectionProgress : CompilerObject, ISection
    {
        public SectionProgress(CompilerContext ctx, string message) : base(ctx)
        {
            m_success = false;
            m_initialMessage = message;
            m_finalMessage = null;
            m_stopwatch = new Stopwatch();
            m_stopwatch.Start();
            Logger.Info($"{m_initialMessage} ...");
        }

        /// <inheritDoc />
        public void Done(string message)
        {
            m_success = true;
            m_finalMessage = message;
        }

        /// <inheritDoc />
        public void Report()
        {
            m_stopwatch.Stop();
            if (m_success)
            {
                Logger.Info(FormatFinalMessage());
            }
            else
            {
                Logger.Error(FormatFinalMessage());
            }
        }

        private string FormatFinalMessage()
        {
            var stringBuilder = new StringBuilder();

            if (string.IsNullOrEmpty(m_finalMessage) || m_initialMessage == m_finalMessage)
            {
                stringBuilder.Append(m_success ? "Done " : "Failed ");
                stringBuilder.Append(char.ToLower(m_initialMessage[0]) + m_initialMessage.Substring(1));
            }
            else
            {
                stringBuilder.Append(m_finalMessage);
            }

            stringBuilder.Append($" (took {m_stopwatch.ElapsedMilliseconds} ms)");
            return stringBuilder.ToString();
        }

        /// <summary>
        /// Did we fail?
        /// </summary>

        private bool m_success;

        /// <summary>
        /// Initial message
        /// </summary>
        private string m_initialMessage;

        /// <summary>
        /// Final message
        /// </summary>
        private string m_finalMessage;

        /// <summary>
        /// StopWatch for time measurement
        /// </summary>
        private Stopwatch m_stopwatch;
    }
}
