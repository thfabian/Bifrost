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

namespace Bifrost.Compiler.Logger
{
    /// <summary>
    /// Severity of a log message 
    /// </summary>
    public enum LogSeverity
    {
        /// <summary>
        /// Debug level is for information that the user shouldn't usually see, but is very useful for debugging problems.
        /// </summary>
        Debug,

        /// <summary>
        /// General status updates that user should see.
        /// </summary>
        Info,

        /// <summary>
        /// Warning: something went wrong, but it's not really a problem.
        /// </summary>
        Warn,

        /// <summary>
        /// Something went wrong but we can still continue.
        /// </summary>
        Error,

        /// <summary>
        /// Something went terribly wrong, always aborts the current work.
        /// </summary>
        Fatal,
    }

    /// <summary>
    /// Logging interface
    /// </summary>
    public interface ILogger
    {
        void Debug(string message);
        void Debug(string message, Exception exception);
        void DebugFormat(string format, params object[] args);

        void Info(string message);
        void Info(string message, Exception exception);
        void InfoFormat(string format, params object[] args);

        void Warn(string message);
        void Warn(string message, Exception exception);
        void WarnFormat(string format, params object[] args);

        void Error(string message);
        void Error(string message, Exception exception);
        void ErrorFormat(string format, params object[] args);

        void Fatal(string message);
        void Fatal(string message, Exception exception);
        void FatalFormat(string format, params object[] args);
    }
}
