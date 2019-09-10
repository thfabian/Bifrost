using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Bifrost.Compiler.Logger
{
    /// <summary>
    /// Log to console
    /// </summary>
    public sealed class Console : Base
    {
        /// <summary>
        /// Write the logging output to the given stream
        /// </summary>
        public TextWriter Stream { get; set; } = System.Console.Error;

        /// <summary>
        /// Use colors in the output?
        /// </summary>
        public bool UseColors { get; set; } = true;

        public override void Sink(LogEntry entry)
        {
            if (UseColors)
            {
                switch (entry.Severity)
                {
                    case LogSeverity.Debug:
                        System.Console.ForegroundColor = ConsoleColor.Gray;
                        break;
                    case LogSeverity.Info:
                        System.Console.ForegroundColor = ConsoleColor.White;
                        break;
                    case LogSeverity.Warn:
                        System.Console.ForegroundColor = ConsoleColor.Yellow;
                        break;
                    case LogSeverity.Error:
                        System.Console.ForegroundColor = ConsoleColor.Red;
                        break;
                    default:
                        break;
                }
            }

            var format = "[{0}] [{1}] [{2}] {3}";
            void sink(params object[] args)
            {
                Stream.WriteLine(string.Format(format, args));
            }

            sink(string.Format("{0,-5}", 
                entry.Severity.ToString().ToUpper()), 
                entry.Time.ToString("yyyy-MM-dd hh:mm:ss,fff"),
                entry.TaskId == null ? string.Format("T{0,2}", entry.ThreadId) : string.Format("t{0,2}", entry.TaskId),
                entry.Message);

            if (UseColors)
            {
                System.Console.ResetColor();
            }
        }
    }
}
