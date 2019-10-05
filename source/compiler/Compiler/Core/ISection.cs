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
    /// Section of code which can be instrumented (e.g for profiling or logging progress)
    /// </summary>
    public interface ISection
    {
        /// <summary>
        /// Called upon successful completion of the section - this will not be called if an exception is thrown in the section
        /// </summary>
        void Done(string message);

        /// <summary>
        /// Output final report - guaranteed to be called
        /// </summary>
        void Report();
    }
}
