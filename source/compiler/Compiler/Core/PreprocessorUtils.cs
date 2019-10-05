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
                // Extract a new macro: #define <name> <value>
                if (token == "#define")
                {
                    var n = 1; // Skip #define and the following whitespace(s)

                    var name = tokenStream.Peak(n);
                    if (macrosToParse.Contains(name))
                    {
                        n += 2; // Skip macro name and the following whitespace
                        var value = "";
                        while (true)
                        {
                            var curToken = tokenStream.Peak(n++);
                            bool endOfMacroValue = false;

                            bool skipNextNewLine = false;
                            for (int i = 0; i < curToken.Length; ++i)
                            {
                                char c = curToken[i];
                                if (c == '\\')
                                {
                                    skipNextNewLine = true;
                                }
                                else if (c == '\n')
                                {
                                    if (skipNextNewLine)
                                    {
                                        skipNextNewLine = false;
                                    }
                                    else
                                    {
                                        endOfMacroValue = true;
                                    }
                                }
                            }

                            if (endOfMacroValue)
                            {
                                break;
                            }
                            else
                            {
                                value += curToken;
                            }
                        }

                        tokenStream.AddMacro(name, value);
                        tokenStream.Consume(n);
                    }
                }
                else
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
                    if (char.IsWhiteSpace(c))
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
                if (m_macros.ContainsKey(name))
                {
                    m_macros[name] = value;
                }
                else
                {
                    m_macros.Add(name, value);
                }
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
                        var prevIndex = 0;
                        var curIndex = expandedToken.IndexOf(name);
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
                                curIndex += value.Length;
                                expansionFound = true;
                            }
                            else
                            {
                                curIndex += name.Length;
                            }

                            prevIndex = curIndex;
                            curIndex = token.IndexOf(name, curIndex);
                        }
                    }
                } while (expansionFound);
                return expandedToken;
            }

            private readonly List<string> m_tokens;

            private readonly Dictionary<string, string> m_macros;

            private int m_curIndex = 0;
        }
    }
}
