using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Bifrost.Compiler.Logger
{
    /// <summary>
    /// Base class of all loggers
    /// </summary>
    public abstract class Base : ILogger
    {
        /// <summary>
        /// Single log entry
        /// </summary>
        public struct LogEntry
        {
            /// <summary>
            /// Severity of the message
            /// </summary>
            public LogSeverity Severity;

            /// <summary>
            /// Message itself
            /// </summary>
            public string Message;

            /// <summary>
            /// Time-stamp
            /// </summary>
            public DateTime Time;

            /// <summary>
            /// Managed ID of the current thread
            /// </summary>
            public int ThreadId;

            /// <summary>
            /// ID of the current task
            /// </summary>
            public int? TaskId;
        }

        /// <summary>
        /// Sink the log entry
        /// </summary>
        public abstract void Sink(LogEntry entry);

        #region ILogger
        public void Debug(string message)
        {
            LogImpl(LogSeverity.Debug, message);
        }

        public void Debug(string message, Exception exception)
        {
            Debug(message + "\n" + exception.ToString());
        }

        public void DebugFormat(string format, params object[] args)
        {
            Debug(string.Format(format, args));
        }

        public void Info(string message)
        {
            LogImpl(LogSeverity.Info, message);
        }

        public void Info(string message, Exception exception)
        {
            Info(message + "\n" + exception.ToString());
        }

        public void InfoFormat(string format, params object[] args)
        {
            Info(string.Format(format, args));
        }

        public void Warn(string message)
        {
            LogImpl(LogSeverity.Warn, message);
        }

        public void Warn(string message, Exception exception)
        {
            Warn(message + "\n" + exception.ToString());
        }

        public void WarnFormat(string format, params object[] args)
        {
            Warn(string.Format(format, args));
        }

        public void Error(string message)
        {
            LogImpl(LogSeverity.Error, message);
        }

        public void Error(string message, Exception exception)
        {
            Error(message + "\n" + exception.ToString());
        }

        public void ErrorFormat(string format, params object[] args)
        {
            Error(string.Format(format, args));
        }
        #endregion

        private void LogImpl(LogSeverity severity, string message)
        {
            lock (m_lock)
            {
                Sink(new LogEntry()
                {
                    Severity = severity,
                    Message = message,
                    Time = DateTime.UtcNow,
                    ThreadId = System.Threading.Thread.CurrentThread.ManagedThreadId,
                    TaskId = Task.CurrentId
                });
            }
        }

        private readonly object m_lock = new object();
    }
}
