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
using System.Diagnostics;
using System.IO;

namespace Bifrost.Compiler.Core
{
    /// <summary>
    /// Internal profiler
    /// </summary>
    public class Profiler : CompilerObject
    {
        public Profiler(CompilerContext ctx) : base(ctx)
        {
        }

        /// <summary>
        /// Create a profiling section
        /// </summary>
        public ISection CreateProfileSection(string identifier)
        {
            m_sections.Add(new ProfileSection(this, identifier, Level++));
            return m_sections[m_sections.Count - 1];
        }

        /// <summary>
        /// Report the profiling results to stdout
        /// </summary>
        public void Report()
        {
            foreach (var section in m_sections)
            {
                Console.WriteLine(string.Format("{0,-40} {1,-10} ms",
                                                new string(' ', section.Level * 2) + section.Identifier,
                                                new string(' ', section.Level * 2) + string.Format("{0,5}", section.ElapsedMilliseconds)));
            }
        }

        public class ProfileSection : ISection
        {
            /// <summary>
            /// Identifier
            /// </summary>
            public string Identifier;

            /// <summary>
            /// Level of intendation
            /// </summary>
            public int Level;

            /// <summary>
            /// Did we already report the result?
            /// </summary>
            private bool m_reported = false;

            /// <summary>
            /// Reference to the profiler
            /// </summary>
            private readonly Profiler m_profiler = null;

            /// <summary>
            /// Stop watch
            /// </summary>
            private Stopwatch m_stopwatch = new Stopwatch();

            public ProfileSection(Profiler profiler, string identifier, int level)
            {
                m_stopwatch.Start();
                m_profiler = profiler;
                Identifier = identifier;
                Level = level;
            }

            /// <inheritDoc />
            public void Done(string message)
            {
                Report();
            }

            /// <inheritDoc />
            public void Report()
            {
                if (!m_reported)
                {
                    m_stopwatch.Stop();
                    m_profiler.Level--;
                    m_reported = true;
                }
            }

            /// <summary>
            /// Get the elapsed ms
            /// </summary>
            public long ElapsedMilliseconds => m_stopwatch.ElapsedMilliseconds;
        }

        /// <summary>
        /// Curent level of profiling
        /// </summary>
        public int Level = 0;

        /// <summary>
        /// Individual sections
        /// </summary>
        private readonly List<ProfileSection> m_sections = new List<ProfileSection>();
    }
}
