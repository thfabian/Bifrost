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
using System.Linq;

namespace Bifrost.Compiler.Core
{
    /// <summary>
    /// C Preprocessor utilities
    /// </summary>
    static public class PreprocessorUtils
    {
        /// <summary>
        /// Parse any macro definitions "#define key value" in <paramref name="input"/> and expand them
        /// </summary>
        /// <param name="input">Input string to parse</param>
        /// <param name="macrosToParse">Only parse and expand macros of the following names</param>
        /// <param name="predefinedMacros">Predefined macros to always expand</param>
        public static string ExpandMacros(string input, List<string> macrosToParse, Dictionary<string, string> predefinedMacros = null)
        {
            var tokenStream = new MacroTokenStream(Tokenize(input), predefinedMacros ?? new Dictionary<string, string>());
            var expandedTokens = new List<string>();

            var token = tokenStream.Next();
            while (token != null)
            {
                bool expandedTokensUpdated = false;

                // Extract a new macro: #define <name> <value>
                if (token == "#define")
                {
                    var n = 1; // Skip #define and the following whitespace(s)

                    var name = tokenStream.Peak(n);
                    if (macrosToParse.Contains(name))
                    {
                        n += 2; // Skip macro name and the following whitespace
                        var value = "";
                        bool skipNextNewLine = false;

                        // We consume as many tokens as the <value> until we find a "\n" character - if we have a "\" character we skip the next newline
                        bool endOfMacroValue = false;
                        while (!endOfMacroValue)
                        {
                            var curToken = tokenStream.Peak(n++);
                            if (curToken == null)
                            {
                                break;
                            }

                            var backSlashIndices = new HashSet<int>();

                            int i = 0;
                            for (; !endOfMacroValue && i < curToken.Length; ++i)
                            {
                                char c = curToken[i];

                                // "\" indicates skipping the next newline - we record the position to be able to remove them later on.
                                if (c == '\\')
                                {
                                    skipNextNewLine = true;
                                    backSlashIndices.Add(i);
                                }
                                else if (c == '\n')
                                {
                                    if (skipNextNewLine)
                                    {
                                        skipNextNewLine = false;
                                    }
                                    else
                                    {
                                        // We found the end of <value>. Our tokenizer is greedy and puts everything that is not a character in a single token, hence we need
                                        // to append everything up to the newline to the <value> and keep everything after the newline.
                                        endOfMacroValue = true;
                                        break;
                                    }
                                }
                            }

                            // Add all the characters up to the newline
                            var leftSubTokenBuilder = new StringBuilder();
                            for (int j = 0; j < i; ++j)
                            {
                                if (!backSlashIndices.Contains(j))
                                {
                                    leftSubTokenBuilder.Append(curToken[j]);
                                }
                            }
                            var leftSubToken = leftSubTokenBuilder.ToString();

                            if (!string.IsNullOrEmpty(leftSubToken))
                            {
                                value += leftSubToken;
                            }

                            if (endOfMacroValue)
                            {
                                // Verbatimly add everything after the newline
                                var rightSubToken = i + 1 > curToken.Length ? null : curToken.Substring(i + 1);
                                if (!string.IsNullOrEmpty(rightSubToken))
                                {
                                    expandedTokens.Add(rightSubToken);
                                }
                            }
                        }

                        tokenStream.AddMacro(name, value);
                        tokenStream.Consume(n);
                        expandedTokensUpdated = true;
                    }
                }

                if (!expandedTokensUpdated)
                {
                    expandedTokens.Add(token);
                }

                token = tokenStream.Next();
            }
            return string.Join("", expandedTokens);
        }

        /// <summary>
        /// Tokenize the string <paramref name="input"/>
        /// </summary>
        public static IEnumerable<string> Tokenize(string input)
        {
            var tokens = new List<string>();

            int curIndex = 0;
            int prevIndex = 0;

            bool insideString = false;
            bool insideWhitespace = false;

            void AddTokenIfInsideWhitespace(bool valueOfInsideWhitespace)
            {
                if (insideWhitespace == valueOfInsideWhitespace)
                {
                    tokens.Add(input.Substring(prevIndex, curIndex - prevIndex));
                    prevIndex = curIndex;
                    insideWhitespace = !insideWhitespace;
                }
            }

            var separator = new List<char>() { '.', ',', ';', '+', '-', '*', '/', '=', '<', '>', '{', '}', '[', ']', '(', ')', ':', '?', '!' };

            for (; curIndex < input.Length; ++curIndex)
            {
                var c = input[curIndex];

                if (c == '"')
                {
                    AddTokenIfInsideWhitespace(true);

                    if (curIndex > 0)
                    {
                        if (input[curIndex - 1] != '\\')
                        {
                            insideString = !insideString;
                        }
                    }
                    else
                    {
                        insideString = !insideString;
                    }
                }
                else if (!insideString)
                {
                    if (char.IsWhiteSpace(c) || separator.Contains(c))
                    {
                        AddTokenIfInsideWhitespace(false);
                    }
                    else
                    {
                        AddTokenIfInsideWhitespace(true);
                    }
                }
            }

            if (curIndex != prevIndex)
            {
                tokens.Add(input.Substring(prevIndex, curIndex - prevIndex));
            }
            return tokens;
        }

        private class MacroTokenStream
        {
            public MacroTokenStream(IEnumerable<string> tokens, Dictionary<string, string> macros)
            {
                m_tokens = new List<string>(tokens);
                m_macros = macros;
                m_curIndex = 0;
            }

            /// <summary>
            /// Get the next token or null if no more tokens are available
            /// </summary>
            public string Next()
            {
                if (m_curIndex < m_tokens.Count)
                {
                    return Expand(m_tokens[m_curIndex++]);
                }
                else
                {
                    return null;
                }
            }

            /// <summary>
            /// Peak the next n-th value or null if no more tokens are available
            /// </summary>
            public string Peak(int n = 1)
            {
                if (m_curIndex + n < m_tokens.Count)
                {
                    return Expand(m_tokens[m_curIndex + n]);
                }
                else
                {
                    return null;
                }
            }

            /// <summary>
            /// Consume <paramref name="num"/> tokens
            /// </summary>
            public void Consume(int n = 1)
            {
                m_curIndex += n;
            }

            /// <summary>
            /// Add a new macro to expand
            /// </summary>
            public void AddMacro(string name, string value)
            {
                m_macros.Update(name, value);
            }

            private string Expand(string token)
            {
                string expandedToken = token;
                bool expansionFound;

                do
                {
                    expansionFound = false;
                    foreach (var (name, value) in m_macros)
                    {
                        // The reason we can't do direct value comparison is that we may expand a macro into several tokens
                        var prevIndex = 0;

                        var curIndex = TokenMatches(expandedToken, name);
                        bool insideString = false;

                        while (curIndex != -1)
                        {
                            // Check that we are not inside a string
                            for (int i = prevIndex; i < curIndex; ++i)
                            {
                                char c = expandedToken[i];

                                if (c == '"')
                                {
                                    if (curIndex > 0)
                                    {
                                        if (expandedToken[curIndex - 1] != '\\')
                                        {
                                            insideString = !insideString;
                                        }
                                    }
                                    else
                                    {
                                        insideString = !insideString;
                                    }
                                }
                            }

                            if (!insideString)
                            {
                                expandedToken = expandedToken.Substring(0, curIndex) + value + expandedToken.Substring(curIndex + name.Length);
                                expansionFound = true;
                            }
                            else
                            {
                                curIndex += name.Length;
                            }

                            prevIndex = curIndex;
                            curIndex = TokenMatches(expandedToken, name, curIndex);
                        }
                    }
                } while (expansionFound);
                return expandedToken;
            }

            /// <summary>
            /// Check if <paramref name="token"/> contains <paramref name="value"/>
            /// </summary>
            static private int TokenMatches(string token, string value, int startIndex = 0)
            {
                var idx = token.IndexOf(value, startIndex);
                if (idx == -1)
                {
                    return idx;
                }

                // Check that the character before the value is a whitespace/newline
                if (idx > 0 && !char.IsWhiteSpace(token[idx - 1]))
                {
                    return -1;
                }

                // Check that the character after the value is a whitespace/newline
                if ((idx + value.Length) != token.Length && !char.IsWhiteSpace(token[idx + value.Length]))
                {
                    return -1;
                }

                return idx;
            }

            private readonly List<string> m_tokens;

            private readonly Dictionary<string, string> m_macros;

            private int m_curIndex = 0;
        }
    }
}
