using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Bifrost.Compiler.Logger
{
    /// <summary>
    /// Forward all logging message to the registered loggers
    /// </summary>
    public class Forward : Logger.Base
    {
        /// <summary>
        /// Registered logger
        /// </summary>
        public Dictionary<string, Logger.Base> Loggers { get; set; } = new Dictionary<string, Logger.Base>();

        public override void Sink(LogEntry entry)
        {
            foreach (var logger in Loggers.Values)
            {
                logger.Sink(entry);
            }
        }

        /// <summary>
        /// Enable/Disable buffer of the log messages
        /// </summary>
        public void EnableBuffering(bool enable)
        {
            if (enable)
            {
                Loggers.Add(BufferTag, new Logger.Buffered());
            }
            else
            {
                Loggers.Remove(BufferTag);
            }
        }

        /// <summary>
        /// Flush all the buffered log messages to the registered loggers
        /// </summary>
        public void FlushBuffer()
        {
            if (!Loggers.ContainsKey(BufferTag))
            {
                return;
            }

            var bufferedLogger = Loggers[BufferTag] as Logger.Buffered;
            bufferedLogger.Flush(Loggers.Where(kvp => kvp.Key != BufferTag || m_flushedLoggerTags.Contains(kvp.Key)).Select(kvp => kvp.Value));

            foreach (var tag in Loggers.Keys)
            {
                m_flushedLoggerTags.Add(tag);
            }
        }

        private HashSet<string> m_flushedLoggerTags = new HashSet<string>();

        private static readonly string BufferTag = "__buffer__";
    }
}
