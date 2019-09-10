using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Bifrost.Compiler.Core
{
    /// <summary>
    /// Report errors to console
    /// </summary>
    public class ErrorReporter : CompilerObject
    {
        public ErrorReporter(CompilerContext ctx) : base(ctx)
        {
        }

        /// <summary>
        /// Use colors in the output?
        /// </summary>
        public bool UseColors { get; set; } = true;

        /// <summary>
        /// Issue an error message to stderr
        /// </summary>
        public void Error(string error)
        {
            Logger.Error(error);
            ErrorImpl(error);
        }

        public void Error(Exception exception)
        {
            Logger.Error($"{exception.Message}");
            Logger.Debug($"{exception.StackTrace}");
            ErrorImpl(exception.Message);
        }

        private void ErrorImpl(string message)
        {
            if (UseColors)
            {
                Console.ForegroundColor = ConsoleColor.White;
                Console.Write("bfc: ");
                Console.ForegroundColor = ConsoleColor.Red;
                Console.Write("error");
                Console.ForegroundColor = ConsoleColor.White;
                Console.Write(": ");
                Console.ResetColor();
                Console.WriteLine(message);
            }
            else
            {
                Console.WriteLine($"bfc: error: {message}");
            }
        }
    }
}
