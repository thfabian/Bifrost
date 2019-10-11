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
using System.IO;
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

        /// <summary>
        /// Get the path to the assembly executable
        /// </summary>
        public static string Executable => Assembly.Location;

        /// <summary>
        /// Get the content of the file in "Embedded/"
        /// </summary>
        public static string GetEmbeddedResource(string filename)
        {
            var embeddedFile = "Bifrost.Compiler." + filename;
            using (Stream stream = Assembly.GetManifestResourceStream(embeddedFile))
            {
                if (stream == null)
                {
                    throw new Exception($"invalid embedded file \"{embeddedFile}\": no such file");
                }

                using (StreamReader reader = new StreamReader(stream))
                {
                    return reader.ReadToEnd();
                }
            }
        }
    }
}
