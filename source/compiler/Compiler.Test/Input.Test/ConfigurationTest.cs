using System;
using System.Collections.Generic;
using System.Text;
using Xunit;
using Xunit.Abstractions;
using Bifrost.Compiler.Test.Utils;

namespace Bifrost.Compiler.Test.Input.Test
{
    public class ConfigurationTest : CompilerTest
    {
        public ConfigurationTest(ITestOutputHelper output) : base(output)
        {
        }

        [Theory]
        [File("ConfigurationTest.File.yaml")]
        public void File(string file)
        {
            var config = CreateConfiguration(file);
            Assert.Equal("foo", config.Clang.Arguments);
            Assert.Equal(new Dictionary<string, string>() { ["foo"] = "bar", ["bar"] = "foo" }, config.Clang.Defines);
            Assert.Equal(new List<string>() { "foo", "bar" }, config.Clang.Includes);
        }

        [Theory]
        [File("ConfigurationTest.CommandLine.yaml")]
        public void CommandLine(string file)
        {
            var config = CreateConfiguration(file, "--clang-arg", "foobar", "-D", "s=4", "-I", "foobar");
            Assert.Equal("foo foobar", config.Clang.Arguments);
            Assert.Equal(new Dictionary<string, string>() { ["foo"] = "bar", ["bar"] = "foo", ["s"] = "4" }, config.Clang.Defines);
            Assert.Equal(new List<string>() { "foo", "bar", "foobar" }, config.Clang.Includes);
        }
    }
}
