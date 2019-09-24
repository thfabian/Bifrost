using System;
using System.Collections.Generic;
using System.Text;

namespace Bifrost.Compiler.Core
{
    /// <summary>
    /// Version information of the compiler
    /// </summary>
    public static class AssemblyInfo
    {
        /// <summary>
        /// Access the version triple Major.Minor.Patch.Build
        /// </summary>
        public static Version Version => Assembly.GetName().Version;

        /// <summary>
        /// Name of the assembly
        /// </summary>
        public static string Name => Assembly.GetName().Name;

        /// <summary>
        /// Assembly object
        /// </summary>
        public static System.Reflection.Assembly Assembly => typeof(AssemblyInfo).Assembly;
    }
}
