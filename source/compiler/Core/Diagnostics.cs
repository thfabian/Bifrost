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
using System.Text;
using System.IO;
using System.Linq;

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
        public void Error(string message, SourceRange range = null)
        {
            Consume(SeverityEnum.Error, range, message, null);
        }

        public void Error(CompilerError error)
        {
            Consume(SeverityEnum.Error, error.Range, error.Message, error.StackTrace);
        }

        public void Error(Exception error)
        {
            Consume(SeverityEnum.Error, null, error.Message, error.StackTrace);
        }

        /// <summary>
        /// Issue a fatal error message
        /// </summary>
        public void Fatal(string message, SourceRange range = null)
        {
            Consume(SeverityEnum.Fatal, range, message, null);
        }

        public void Fatal(CompilerError error)
        {
            Consume(SeverityEnum.Fatal, error.Range, error.Message, error.StackTrace);
        }

        public void Fatal(Exception error)
        {
            Consume(SeverityEnum.Fatal, null, error.Message, error.StackTrace);
        }


        private enum SeverityEnum
        {
            Warning,
            Error,
            Fatal
        }

        private void Consume(SeverityEnum severity, SourceRange range, string message, string stackTrace)
        {
            var locStart = range?.Start;
            var locStartStr = locStart != null ? locStart.ToString() + " " : "";

            // Log the message
            switch (severity)
            {
                case SeverityEnum.Warning:
                    Logger.Warn($"{locStartStr}{message}");
                    break;
                case SeverityEnum.Error:
                    Logger.Error($"{locStartStr}{message}");
                    break;
                case SeverityEnum.Fatal:
                    Logger.Fatal($"{locStartStr}{message}");
                    break;
            }

            if (!string.IsNullOrEmpty(stackTrace))
            {
                Logger.Debug(stackTrace);
            }

            // Print to console
            if (UseColors)
            {
                Console.ForegroundColor = ConsoleColor.White;
                Console.Write($"{GetLocationOrExecutable(locStart)} ");

                switch (severity)
                {
                    case SeverityEnum.Warning:
                        Console.ForegroundColor = ConsoleColor.Magenta;
                        break;
                    case SeverityEnum.Error:
                        Console.ForegroundColor = ConsoleColor.Red;
                        break;
                    case SeverityEnum.Fatal:
                        Console.ForegroundColor = ConsoleColor.Red;
                        break;
                }

                Console.Write(GetNameOfSeverity(severity) + ": ");
                Console.ForegroundColor = ConsoleColor.White;
                Console.WriteLine(message);
                Console.ResetColor();
            }
            else
            {
                Console.WriteLine($"{GetLocationOrExecutable(locStart)} {GetNameOfSeverity(severity)}: {message}");
            }

            // Render the code location
            if (locStart != null && locStart.Row != -1 && locStart.Column != -1 && File.Exists(locStart.File))
            {
                var lines = File.ReadAllLines(locStart.File);
                if (locStart.Row - 1 < lines.Count())
                {
                    var width = Console.WindowWidth - 1;

                    var line = lines[locStart.Row - 1];
                    var size = line.Count();

                    if (size > width)
                    {
                        // Chop the line if necessary
                        var col = locStart.Column;

                        if (col < 5)
                        {
                            // Chop right
                            var left = line.Substring(0, col);
                            var right = line.Substring(col, Math.Min(size, width) - col - 4) + " ...";

                            Console.WriteLine(left + right);
                        }
                        else if (size - col < 5)
                        {
                            // Chop right

                        }
                        else
                        {
                            // Chop both ends

                        }
                    }
                    else
                    {
                        Console.WriteLine(line);
                    }
                }
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

        private string GetNameOfSeverity(SeverityEnum severity)
        {
            switch (severity)
            {
                case SeverityEnum.Warning:
                    return "warning";
                case SeverityEnum.Error:
                    return "error";
                case SeverityEnum.Fatal:
                    return "fatal error";
            }
            return "error";
        }
    }
}
