using System;
using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;

namespace Bifrost.Compiler.Core
{
    /// <summary>
    /// Match a string containing wildcards (*)
    /// </summary>
    public class StringMatcher
    {
        private readonly string[] m_subMatches = new string[] { "" };

        private readonly Regex m_regex = null;

        public StringMatcher(string match)
        {
            if (!string.IsNullOrEmpty(match))
            {
                m_subMatches = match.Split('*');
                m_regex = new Regex("^" + match.Replace("*", ".*?") + "$");
            }
        }

        /// <summary>
        /// Checks if the string <paramref name="input"/> matches
        /// </summary>
        public bool IsMatch(string input)
        {
            // Quick checks
            if (m_subMatches.Length == 1)
            {
                return m_subMatches[0] == input;
            }

            if (m_subMatches.Length == 2)
            {
                bool startsWithWildcard = m_subMatches[0] == "";
                bool endsWithWildcard = m_subMatches[1] == "";

                if (!startsWithWildcard && !endsWithWildcard)
                {
                    return input.StartsWith(m_subMatches[0]) && input.EndsWith(m_subMatches[1]);
                }
                else if (startsWithWildcard && !endsWithWildcard)
                {
                    return input.EndsWith(m_subMatches[1]);
                }
                else if (!startsWithWildcard && endsWithWildcard)
                {
                    return input.StartsWith(m_subMatches[0]);
                }
                else // this is "*"
                {
                    return true;
                }
            }

            // Regex check
            return m_regex.IsMatch(input);
        }
    }
}
