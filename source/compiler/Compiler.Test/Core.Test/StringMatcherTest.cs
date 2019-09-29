using Bifrost.Compiler.Core;
using System;
using System.Collections.Generic;
using System.Text;
using Xunit;

namespace Bifrost.Compiler.Test.Core.Test
{
    public class StringMatcherTest
    {
        [Theory]
        // Single wildcard
        [InlineData("", "", true)]
        [InlineData("", "foo", false)]
        [InlineData("foo", "foo", true)]
        [InlineData("fo", "foo", false)]
        [InlineData("bar", "foo", false)]

        [InlineData("*", "", true)]
        [InlineData("*", "foo", true)]

        [InlineData("foo*", "foo", true)]
        [InlineData("foo*", "foobar", true)]
        [InlineData("foo*", "fob", false)]

        [InlineData("*foo", "foo", true)]
        [InlineData("*foo", "fob", false)]
        [InlineData("*foo", "foob", false)]
        [InlineData("*foo", "barfoo", true)]

        [InlineData("foo*bar", "foobar", true)]

        // Multi wildcard
        [InlineData("foo*bar*", "foobar", true)]
        [InlineData("foo*bar*", "foobar2", true)]
        [InlineData("foo*bar*", "fooba2", false)]
        [InlineData("foo*bar*foo", "foo2barfoo", true)]
        [InlineData("foo*bar*foo", "foo2bar2foo", true)]
        [InlineData("foo*bar*foo", "foo2bar2foo2", false)]

        public void Match(string matchString, string inputString, bool isMatch)
        {
            var matcher = new StringMatcher(matchString);
            Assert.Equal(matcher.IsMatch(inputString), isMatch);
        }
    }
}
