using Bifrost.Compiler.Core;
using System;
using System.Collections.Generic;
using System.Text;
using System.Linq;
using Xunit;

namespace Bifrost.Compiler.Test.Core.Test
{
    public class PreprocessorUtilsTest
    {
        [Theory]
        [InlineData("foo bar", new[] { "foo", " ", "bar" })]
        [InlineData("foo bar ", new[] { "foo", " ", "bar", " " })]
        [InlineData("foo\nbar", new[] { "foo", "\n", "bar" })]

        [InlineData("foo bar    bar", new[] { "foo", " ", "bar", "    ", "bar" })]
        [InlineData("foo bar\r\n foo", new[] { "foo", " ", "bar", "\r\n ", "foo" })]
        [InlineData("foo bar\r\n\n\r\n \n foo", new[] { "foo", " ", "bar", "\r\n\n\r\n \n ", "foo" })]

        [InlineData("foo \"bar\"", new[] { "foo", " ", "\"bar\"" })]
        [InlineData("foo \"bar bar\"", new[] { "foo", " ", "\"bar bar\"" })]
        [InlineData("foo \"bar \\\" bar\"", new[] { "foo", " ", "\"bar \\\" bar\"" })]

        [InlineData("bar \"bar\" foo \"bar\"", new[] { "bar", " ", "\"bar\"", " ", "foo", " ", "\"bar\"" })]

        public void Tokenize(string input, IEnumerable<string> tokens)
        {
            Assert.Equal(tokens, PreprocessorUtils.Tokenize(input));
        }

        [Theory]
        [InlineData("foo", "foo", new string[] { })]
        [InlineData("#define FOO foo\nFOO", "foo", new string[] { "FOO" })]
        [InlineData("#define FOO foo \nFOO", "foo", new string[] { "FOO" })]
        //[InlineData("#define FOO foo\n FOO ", " foo ", new string[] { "FOO" })]

        public void ExpandMacros(string input, string expandedInput, IEnumerable<string> macrosToParse, Dictionary<string, string> predefinedMacros = null)
        {
            Assert.Equal(expandedInput, PreprocessorUtils.ExpandMacros(input, macrosToParse.ToList(), predefinedMacros));
        }
    }
}
