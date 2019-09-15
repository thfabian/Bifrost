using System;
using System.Collections.Generic;
using System.Text;

namespace Bifrost.Compiler.Core
{
    /// <summary>
    /// Report errors to console
    /// </summary>
    public class Diagnostics : CompilerObject
    {
        public Diagnostics(CompilerContext ctx) : base(ctx)
        {
        }

        /// <summary>
        /// Use colors in the output?
        /// </summary>
        public bool UseColors { get; set; } = true;

        /// <summary>
        /// Issue an error message
        /// </summary>
        public void Error(string message, SourceLocation location = null)
        {
            Consume(SeverityEnum.Error, location, message, null);
        }

        public void Error(CompilerError error)
        {
            Consume(SeverityEnum.Error, error.Location, error.Message, error.StackTrace);
        }

        public void Error(Exception error)
        {
            Consume(SeverityEnum.Error, null, error.Message, error.StackTrace);
        }

        private enum SeverityEnum
        {
            Warning,
            Error
        }

        private void Consume(SeverityEnum severity, SourceLocation location, string message, string stackTrace)
        {
            switch (severity)
            {
                case SeverityEnum.Warning:
                    Logger.Warn($"{location} {message}");
                    break;
                case SeverityEnum.Error:
                    Logger.Error($"{location} {message}");
                    break;
            }
            if (!string.IsNullOrEmpty(stackTrace))
            {
                Logger.Debug(stackTrace);
            }

            if (UseColors)
            {
                Console.ForegroundColor = ConsoleColor.White;
                Console.Write($"{GetLocationOrExecutable(location)} ");

                switch (severity)
                {
                    case SeverityEnum.Warning:
                        Console.ForegroundColor = ConsoleColor.Magenta;
                        break;
                    case SeverityEnum.Error:
                        Console.ForegroundColor = ConsoleColor.Red;
                        break;
                }
                Console.Write(severity.ToString().ToLower() + ": ");
                Console.ForegroundColor = ConsoleColor.White;
                Console.WriteLine(message);
                Console.ResetColor();
            }
            else
            {
                Console.WriteLine($"{GetLocationOrExecutable(location)} {severity.ToString().ToLower()}: {message}");
            }
        }

        private string GetLocationOrExecutable(SourceLocation location)
        {
            if (location != null)
            {
                return location.ToString();
            }
            else
            {
                return AssemblyInfo.Name + ":";
            }
        }
    }
}
