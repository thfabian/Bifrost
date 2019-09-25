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
    /// A collection of sections
    /// </summary>
    public class SectionCollection : ISection, IDisposable
    {
        public SectionCollection(IEnumerable<ISection> sections)
        {
            m_reported = false;
            m_sections = sections;
        }


        public void Done(string message = null)
        {
            ForEach(s => s.Done(message));
        }

        public void Report()
        {
            if (!m_reported)
            {
                ForEach(s => s.Report());
                m_reported = true;
            }
        }

        public void Dispose()
        {
            Report();
        }

        private void ForEach(Action<ISection> action)
        {
            foreach (var section in m_sections)
            {
                action(section);
            }
        }

        /// <summary>
        /// Did we already report?
        /// </summary>
        private bool m_reported;

        /// <summary>
        /// Collection of sections
        /// </summary>
        private IEnumerable<ISection> m_sections;
    }
}
