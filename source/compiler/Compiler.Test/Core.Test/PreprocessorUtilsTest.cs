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
        // Pass-through
        [InlineData("foo", "foo", new string[] { })]
        [InlineData("#define FOO foo", "#define FOO foo", new string[] { })]
        [InlineData("#define FOO foo", "", new string[] { "FOO" })]

        // Predefined
        [InlineData("FOO", "foo", new string[] { "FOO" }, new string[] { "FOO=foo" })]
        [InlineData("FOO", "FOO", new string[] { "FOO" }, new string[] { "FO=foo" })]

        // Single expansion
        [InlineData("#define FOO foo\nFOO", "foo", new string[] { "FOO" })]
        [InlineData("#define FOO foo \nFOO", "foo", new string[] { "FOO" })]
        [InlineData("#define FOO foo\n FOO ", " foo ", new string[] { "FOO" })]
        [InlineData("#define FOO foo\nbarFOO\nFOO", "barFOO\nfoo", new string[] { "FOO" })]
        [InlineData("#define FOO foo\n FOOs\nFOO", " FOOs\nfoo", new string[] { "FOO" })]

        //[InlineData("#define FOO bar\\\nfoo\nFOO", " bar\nfoo", new string[] { "FOO" })]
        //[InlineData("#define FOO bar \\\n foo\nFOO", " bar \n foo", new string[] { "FOO" })]

        // Multi expansion
        [InlineData("#define FOO foo\n#define BAR FOO\nFOO BAR", "foo foo", new string[] { "FOO", "BAR" })]
        [InlineData("#define FOO foo\n#define BAR FOO FOO\nBAR", "foo foo", new string[] { "FOO", "BAR" })]

        public void ExpandMacros(string input, string expandedInput, IEnumerable<string> macrosToParse, IEnumerable<string> predefinedMacros = null)
        {
            string expected = expandedInput;
            string actual = PreprocessorUtils.ExpandMacros(input, macrosToParse.ToList(),
                predefinedMacros == null ? new Dictionary<string, string>() : predefinedMacros.ToDictionary(s => s.Substring(0, s.IndexOf("=")), s => s.Substring(s.IndexOf("=") + 1)));
            Assert.Equal(expected, actual);
        }
    }
}
