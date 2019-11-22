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
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace Bifrost.Compiler.Core
{
    /// <summary>
    /// Wrapper for clang-format.exe
    /// </summary>
    public class ClangFormat : CompilerObject
    {
        public ClangFormat(CompilerContext ctx) : base(ctx) { }

        /// <summary>
        /// Format the given file
        /// </summary>
        public void Format(string file)
        {
            var clangFormat = Executable;
            if (clangFormat == null)
            {
                Logger.Warn($"Skipping clang format: clang-format executable not found");
            }

            var startInfo = new ProcessStartInfo
            {
                FileName = clangFormat,
                Arguments = "-i -style=\"{BasedOnStyle: Google, ColumnLimit: 160, SortIncludes: false}\" " + $"\"{file}\"",

                // Hide console window
                UseShellExecute = false,
                CreateNoWindow = true
            };

            Logger.Debug($"Running clang format: {startInfo.FileName} {startInfo.Arguments}");
            var process = Process.Start(startInfo);
            process.WaitForExit();

            if (process.ExitCode != 0)
            {
                Logger.Warn($"Failed running clang format on \"{file}\"");
            }
        }

        private string Executable
        {
            get
            {
                var exe = "clang-format" + (RuntimeInformation.IsOSPlatform(OSPlatform.Windows) ? ".exe" : "");
                foreach (var suffixPath in new string[] { ".", "native/win7-x64", "native/mac", "native/unix" })
                {
                    var path = Utils.IO.MakeAbsolute(Path.Combine(Path.GetDirectoryName(AssemblyInfo.Executable), suffixPath, exe));
                    if (File.Exists(path))
                    {
                        return path;
                    }
                }
                return null;
            }
        }
    }
}
