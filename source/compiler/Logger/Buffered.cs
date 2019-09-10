using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.Concurrent;

namespace Bifrost.Compiler.Logger
{
    internal class Buffered : Base
    {
        public override void Sink(LogEntry entry)
        {
            m_queue.Enqueue(entry);
        }

        public void Flush(IEnumerable<Logger.Base> loggers)
        {
            while (m_queue.Count > 0)
            {
                var entry = m_queue.Dequeue();
                foreach (var logger in loggers)
                {
                    logger.Sink(entry);
                }
            }
        }

        private Queue<LogEntry> m_queue = new Queue<LogEntry>();
    }
}
