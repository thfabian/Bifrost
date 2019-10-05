using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using Bifrost.Compiler.Core;
using Bifrost.Compiler.Input;
using Xunit;
using Xunit.Abstractions;

namespace Bifrost.Compiler.Test.Utils
{
    /// <summary>
    /// Base class of all tests
    /// </summary>
    public class CompilerTest
    {
        /// <summary>
        /// Context of the run
        /// </summary>
        readonly CompilerContext Context = new CompilerContext();

        private readonly ITestOutputHelper m_output;

        private readonly CompilerTestLogger m_logger;

        public CompilerTest(ITestOutputHelper output)
        {
            m_output = output;
            m_logger = new CompilerTestLogger(output);

            Context.Logger.Loggers.Add("test_logger", m_logger);
            Context.Diagnostics.UseColors = false;
            Context.Diagnostics.Stream = m_logger.Stream;
        }

        /// <summary>
        /// Create a configuration of the input <paramref name="file"/>
        /// </summary>
        public Configuration CreateConfiguration(string file, params string[] arguments)
        {
            Configuration config = null;

            CommandLine cmd = null;
            try
            {
                cmd = CommandLine.ParseArguments(Context, new List<string>(arguments) { "-l", file }); ;
            }
            catch (Exception e)
            {
                Context.Diagnostics.Fatal(e);
                Assert.True(false, e.Message);
            }

            var builder = new ConfigurationBuilder(Context);
            try
            {
                config = builder.Build(cmd, cmd.Files.FirstOrDefault());
            }
            catch (CompilerError e)
            {
                Context.Diagnostics.Fatal(e);
                Assert.True(false, e.Message);
            }

            return config;
        }
    }
}
