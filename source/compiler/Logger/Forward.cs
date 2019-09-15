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
    }
}
