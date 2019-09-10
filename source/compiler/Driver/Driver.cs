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
            var ctx = new Core.CompilerContext();
            ctx.EnableLogBuffering();

            var errorReporter = new Core.ErrorReporter(ctx);
            int exitCode = 0;

            try
            {
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

                var parserResult = parser.ParseArguments<Input.CommandLine>(args);

                // Run the compiler
                parserResult.WithParsed(c =>
                {
                    exitCode = RunCompiler(ctx, c);
                });
                parserResult.WithNotParsed(errors =>
                {
                    foreach (var err in errors)
                    {
                        if (err is HelpRequestedError)
                        {
                            Console.WriteLine(GetHelp(parserResult));
                        }
                        else if (err is MissingRequiredOptionError e)
                        {
                            throw new Exception($"option --{e.NameInfo.LongName} is required");
                        }
                    }
                });
            }
            catch (Exception e)
            {
                errorReporter.Error(e);
                exitCode = 1;
            }
            return exitCode;
        }

        private static int RunCompiler(Core.CompilerContext ctx, Input.CommandLine cmd)
        {
            var errorReporter = new Core.ErrorReporter(ctx)
            {
                UseColors = !cmd.DisableColors
            };

            try
            {
                if (cmd.Logging)
                {
                    ctx.TryRegisterLogger(new Logger.Console()
                    {
                        UseColors = !cmd.DisableColors
                    });
                    ctx.DisableLogBuffering();
                }

                // 1) Build the configuration
                var configBuilder = new Input.ConfigurationBuilder(ctx);
                var config = configBuilder.Build(cmd, "");

                if (config.General.Logging)
                {
                    ctx.TryRegisterLogger(new Logger.Console());
                    ctx.DisableLogBuffering();
                }

                // 2) Run the configuration
                return ctx.Run(config);
            }
            catch (Exception e)
            {
                errorReporter.Error(e);
                return 1;
            }
        }

        private static string GetHelp<T>(CommandLine.ParserResult<T> parserResult)
        {
            var helpTxt = new HelpText
            {
                AddDashesToOption = true,
                AutoVersion = false,
                MaximumDisplayWidth = Console.WindowWidth,
            };
            helpTxt.AddPreOptionsText("Bifrost Compiler (0.0.1) - Parse and extract hooks from C/C++ files.\n\nOPTIONS:");
            helpTxt.AddOptions(parserResult);
            helpTxt.AddPostOptionsText("EXAMPLES:\n  bfc.exe -f input.yaml");

            return helpTxt.ToString();
        }
    }
}
