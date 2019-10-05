using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Linq;
using Bifrost.Compiler;
using Bifrost.Compiler.Logger;
using Xunit.Abstractions;

namespace Bifrost.Compiler.Test.Utils
{
    /// <summary>
    /// Compiler logger used in the tests which sinks to Debug.Output and Xunit output
    /// </summary>
    public class CompilerTestLogger : Logger.Base, IDisposable
    {
        private readonly Logger.Console m_console;

        public CompilerTestLogger(ITestOutputHelper testOutput)
        {
            Stream = new TestTextWriter(testOutput);
            m_console = new Logger.Console
            {
                Stream = Stream,
                UseColors = false
            };
        }

        public override void Sink(LogEntry entry)
        {
            m_console.Sink(entry);
        }

        public void Dispose()
        {
            Stream.Flush();
        }

        public TestTextWriter Stream { get; }

        public class TestTextWriter : TextWriter, IDisposable
        {
            private StringBuilder m_builder = new StringBuilder();

            private ITestOutputHelper m_testOutput;

            public TestTextWriter(ITestOutputHelper testOutput)
            {
                m_testOutput = testOutput;
            }

            public override void Write(char value)
            {
                if (value == '\n')
                {
                    Flush();
                }
                else
                {
                    m_builder.Append(value);
                }
            }

            public override void Flush()
            {
                var line = m_builder.ToString();
                if (!string.IsNullOrEmpty(line) && NewLine.Length == 2)
                {
                    line = line.Remove(line.Length - 1, 1);
                }
                m_testOutput.WriteLine(line);
                System.Diagnostics.Debug.WriteLine(line);
                m_builder.Clear();
            }

            public override Encoding Encoding => Encoding.Default;
        }
    }
}
