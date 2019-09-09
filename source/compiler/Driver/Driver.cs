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
using CommandLine;
using CommandLine.Text;
using Bifrost.Compiler.Input;

namespace Bifrost.Compiler.Driver
{
    /// <summary>
    /// Invokes the compiler
    /// </summary>
    public static class Driver
    {
        public static int Run(IEnumerable<string> args)
        {
            int exitCode = 0;

            // Parse command-line
            var parser = new Parser(s =>
            {
                s.CaseSensitive = Parser.Default.Settings.CaseSensitive;
                s.CaseInsensitiveEnumValues = Parser.Default.Settings.CaseInsensitiveEnumValues;
                s.ParsingCulture = Parser.Default.Settings.ParsingCulture;
                s.HelpWriter = null;
                s.IgnoreUnknownArguments = Parser.Default.Settings.IgnoreUnknownArguments;
                s.AutoHelp = true;
                s.AutoVersion = false;
                s.EnableDashDash = false;
                s.MaximumDisplayWidth = Parser.Default.Settings.MaximumDisplayWidth;
            });

            var result = parser.ParseArguments<Configuration>(args);
            result.WithParsed(c =>
            {
                Console.WriteLine(c);
            });
            result.WithNotParsed(errors =>
            {
                foreach (var err in errors)
                {
                    if (err.GetType() == typeof(HelpRequestedError))
                    {
                        var helpTxt = new HelpText
                        {
                            AddDashesToOption = true,
                            AutoVersion = false,
                            MaximumDisplayWidth = Console.WindowWidth,
                        };
                        helpTxt.AddPreOptionsText("Bifrost Compiler (0.0.1) - Parse and extract hooks from C++ files.\n\nOPTIONS:");
                        helpTxt.AddOptions(result);
                        helpTxt.AddPostOptionsText("EXAMPLES:\n  bfc.exe --help");

                        Console.WriteLine(helpTxt.ToString());
                    }
                    else
                    {
                        Console.WriteLine(err);
                    }
                }
                exitCode = 1;
            });
            return exitCode;
        }

        private static int RunCompiler(Configuration c)
        {
            return 0;
        }
    }
}
