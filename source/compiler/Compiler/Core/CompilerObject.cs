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
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;
using Bifrost.Compiler.Logger;

namespace Bifrost.Compiler.Core
{
    /// <summary>
    /// Bifrost object
    /// </summary>
    public class CompilerObject
    {
        /// <summary>
        /// Underlying context
        /// </summary>
        public CompilerContext Context;

        /// <summary>
        /// Logger of the context
        /// </summary>
        public ILogger Logger => Context.Logger;

        /// <summary>
        /// Diagnostic reporter
        /// </summary>
        public Diagnostics Diagnostics => Context.Diagnostics;

        /// <summary>
        /// I/O related functions
        /// </summary>
        public IO IO => Context.IO;

        /// <summary>
        /// I/O related functions
        /// </summary>
        public Utils Utils => Context.Utils;

        /// <summary>
        /// Internal Profiler
        /// </summary>
        public Profiler Profiler => Context.Profiler;

        public CompilerObject(CompilerContext ctx)
        {
            Context = ctx;
        }

        /// <summary>
        /// Create a new progress object
        /// </summary>
        public SectionCollection CreateSection(string message)
        {
            var stackTrace = new StackTrace();
            var frame = stackTrace.GetFrame(1);
            return CreateSection(frame.GetMethod().DeclaringType.Name + "." + frame.GetMethod().Name, message);
        }

        public SectionCollection CreateSection(string identifier, string message)
        {
            return new SectionCollection(new List<ISection>()
            {
                new SectionProgress(Context, message),
                Profiler.CreateProfileSection(identifier)
            });
        }
    }
}
