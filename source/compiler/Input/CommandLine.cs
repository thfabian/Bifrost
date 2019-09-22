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
using System.CommandLine;
using System.CommandLine.Builder;
using System.IO;
using static Bifrost.Compiler.Core.DictionaryExtensions;

namespace Bifrost.Compiler.Input
{
    /// <summary>
    /// Command line options which are exposed to the user
    /// </summary>
    public class CommandLine
    {
        /// <summary>
        /// Do we stop the execution after command-line parsing?
        /// </summary>
        public bool Stop { get; set; } = false;

        /// <summary>
        /// Input file(s)
        /// </summary>
        public IEnumerable<string> Files { get; set; } = new List<string>();

        /// <summary>
        /// Root command of the command line parser
        /// </summary>
        public RootCommand RootCommand { get; set; }

        /// <summary>
        /// Result of the command-line parser
        /// </summary>
        public ParseResult ParseResult { get; set; }

        /// <summary>
        /// Update the <paramref name="config"/> with the parsed fields
        /// </summary>
        public void UpdateConfiguration(Core.CompilerContext ctx, Configuration config)
        {
            foreach (var action in m_actions)
            {
                if (action.Stage == OptionAction.StageEnum.ConfigurationBuilder)
                {
                    if (ParseResult.HasOption(action.Option))
                    {
                        action.HandleOption(action.Option, this, ctx, config);
                    }
                }
            }
        }

        public struct OptionAction
        {
            public enum StageEnum
            {
                /// <summary>
                /// Do not handle this option
                /// </summary>
                Skip,

                /// <summary>
                /// Checked right after the command arguments have been parsed
                /// </summary>
                CommandBuilder,

                /// <summary>
                /// Checked during building of the configuration
                /// </summary>
                ConfigurationBuilder,
            }

            public Option Option;

            public StageEnum Stage;

            public Action<Option, CommandLine, Core.CompilerContext, Configuration> HandleOption;
        }

        /// <summary>
        /// Add a new action
        /// </summary>
        public void Add(OptionAction optionAction)
        {
            RootCommand.Add(optionAction.Option);
            m_actions.Add(optionAction);
        }

        /// <summary>
        /// Access the actions
        /// </summary>
        public IEnumerable<OptionAction> Actions => m_actions;

        private List<OptionAction> m_actions = new List<OptionAction>();

        /// <summary>
        /// Parse the command line options <paramref name="args"/> 
        /// </summary>
        static public CommandLine ParseArguments(Core.CompilerContext ctx, IEnumerable<string> args)
        {
            var cmd = new CommandLine
            {
                RootCommand = new RootCommand()
            };

            cmd.Add(AddInput());
            cmd.Add(AddHelp());
            cmd.Add(AddConfig());
            cmd.Add(AddVersion());
            cmd.Add(AddLogging());
            cmd.Add(AddOutput());
            cmd.Add(AddInclues());
            cmd.Add(AddDefines());
            cmd.Add(AddClangArgs());
            cmd.Add(AddNoColor());

            // Build the command line
            CommandLineBuilder builder = new CommandLineBuilder(cmd.RootCommand);

            // Parse the command line
            var parser = builder.Build();
            cmd.ParseResult = parser.Parse(args.ToList());

            // See AddInput for explanation of what we try to do here
            var inputs = cmd.ParseResult.UnmatchedTokens;
            var validInputs = new List<string>();
            foreach (var input in inputs)
            {
                if (!input.StartsWith("-"))
                {
                    validInputs.Add(input);
                }
            }

            foreach (var error in cmd.ParseResult.Errors)
            {
                // Allow additional help aliases
                foreach (var helpVariant in new[] { "/?", "-help" })
                {
                    if (error.Message.Contains($"'{helpVariant}'"))
                    {
                        cmd.Actions.Where(action => action.Option.Name == "help").ToList().ForEach(action => action.HandleOption(action.Option, cmd, ctx, null));
                        if (cmd.Stop)
                        {
                            return cmd;
                        }
                    }
                }

                // Our "inputs" are flagged as error by the library because we use unmatched tokens as position arguments -> don't error on these
                bool isError = true;
                foreach (var input in validInputs)
                {
                    if (error.Message.Contains($"'{input}'"))
                    {
                        isError = false;
                        break;
                    }
                }
                if (isError)
                {
                    throw new Exception(error.ToString());
                }
            }

            // Handle the options
            foreach (var action in cmd.Actions)
            {
                if (action.Stage == OptionAction.StageEnum.CommandBuilder)
                {
                    if (cmd.ParseResult.HasOption(action.Option))
                    {
                        action.HandleOption(action.Option, cmd, ctx, null);
                        if (cmd.Stop)
                        {
                            return cmd;
                        }
                    }
                }
            }

            // Validate options
            if (inputs.Count == 0)
            {
                throw new Exception("no input files");
            }
            cmd.Files = new List<string>(inputs);

            return cmd;
        }

        private static OptionAction AddInput() => new OptionAction()
        {
            // This option is never parsed - because this command line library doesn't really support positional arguments (?) we fake them by:
            //  1) Create this option for the help statement
            //  2) Use the UnmatchedTokens of the ParseResult to access them (note we have to fiddle with the errors as they are flagged as unknown arguments).
            Option = new Option("_inputs_", "Input configuration (.yaml), use --config for details."),
            Stage = OptionAction.StageEnum.Skip,
        };

        private static OptionAction AddHelp() => new OptionAction()
        {
            Option = new Option(new[] { "--help", "-h" }, "Print this help statement."),
            Stage = OptionAction.StageEnum.CommandBuilder,
            HandleOption = (opt, cmd, ctx, cfg) =>
            {
                var console = new CommandLineConsole();
                var helpBuilder = new HelpBuilder(console);
                helpBuilder.Write(cmd.RootCommand);

                var helpStr = $"Bifrost Compiler ({Core.AssemblyInfo.Version}) - Parse and extract hooks from C/C++ files.\n\n";
                helpStr += $"Usage:\n  {Core.AssemblyInfo.Name}.exe [options] <inputs>\n\n";

                var optionString = console.ToString();
                helpStr += optionString.ToString().Substring(optionString.IndexOf("Options:"));
                helpStr = helpStr.Replace("_inputs_", "<inputs>");

                helpStr += $"Example:\n  {Core.AssemblyInfo.Name}.exe input.yaml -o C:/foo/bar";

                Console.WriteLine(helpStr);
                cmd.Stop = true;
            }
        };

        private static OptionAction AddConfig() => new OptionAction()
        {
            Option = new Option(new[] { "--config", "-c" }, "Print details on input configuration."),
            Stage = OptionAction.StageEnum.CommandBuilder,
            HandleOption = (opt, cmd, ctx, cfg) =>
            {
                Console.WriteLine(ConfigurationPrinter.Print(new Configuration()));
                cmd.Stop = true;
            }
        };

        private static OptionAction AddVersion() => new OptionAction()
        {
            Option = new Option(new[] { "--version", "-v" }, "Print version number and exit."),
            Stage = OptionAction.StageEnum.CommandBuilder,
            HandleOption = (opt, cmd, ctx, cfg) =>
            {
                Console.WriteLine(Core.AssemblyInfo.Version);
                cmd.Stop = true;
            }
        };

        private static OptionAction AddLogging() => new OptionAction()
        {
            Option = new Option(new[] { "--logging", "-l" }, "Enable logging."),
            Stage = OptionAction.StageEnum.CommandBuilder,
            HandleOption = (opt, cmd, ctx, cfg) =>
            {
                ctx.Logger.Loggers.Add("console", new Logger.Console());
            }
        };

        private static OptionAction AddNoColor() => new OptionAction()
        {
            Option = new Option(new[] { "--no-color" }, "Disable colored output."),
            Stage = OptionAction.StageEnum.CommandBuilder,
            HandleOption = (opt, cmd, ctx, cfg) =>
            {
                ctx.Diagnostics.UseColors = false;
                if (ctx.Logger.Loggers.ContainsKey("console"))
                {
                    (ctx.Logger.Loggers["console"] as Logger.Console).UseColors = false;
                }
            }
        };

        private static OptionAction AddOutput()
        {
            var argument = new Argument
            {
                ArgumentType = typeof(string),
                Arity = ArgumentArity.ExactlyOne,
                Name = "dir",
            };
            argument.SetDefaultValue(Directory.GetCurrentDirectory());

            return new OptionAction()
            {
                Option = new Option(new[] { "--output", "-o" }, "Write output to directory.")
                {
                    Argument = argument
                },
                Stage = OptionAction.StageEnum.ConfigurationBuilder,
                HandleOption = (opt, cmd, ctx, cfg) =>
                {
                    var output = cmd.ParseResult.ValueForOption<string>("output");
                }
            };
        }

        private static OptionAction AddInclues()
        {
            var argument = new Argument
            {
                ArgumentType = typeof(string),
                Arity = ArgumentArity.OneOrMore,

                Name = "dir",
            };
            argument.SetDefaultValue(Array.Empty<string>());

            return new OptionAction()
            {
                Option = new Option(new[] { "--include", "-I" }, "Add directory to include search path.")
                {
                    Argument = argument
                },
                Stage = OptionAction.StageEnum.ConfigurationBuilder,
                HandleOption = (opt, cmd, ctx, cfg) =>
                {
                    cfg.Clang.Includes.AddRange(cmd.ParseResult.ValueForOption<string[]>("include"));
                }
            };
        }

        private static OptionAction AddDefines()
        {
            var argument = new Argument
            {
                ArgumentType = typeof(string),
                Arity = ArgumentArity.OneOrMore,
                Name = "macro>=<value",
            };
            argument.SetDefaultValue(Array.Empty<string>());

            return new OptionAction()
            {
                Option = new Option(new[] { "--define", "-D" }, "Define <macro> to <value> (or 1 if <value> is omitted).")
                {
                    Argument = argument
                },
                Stage = OptionAction.StageEnum.ConfigurationBuilder,
                HandleOption = (opt, cmd, ctx, cfg) =>
                {
                    foreach (var define in cmd.ParseResult.ValueForOption<string[]>("define"))
                    {
                        var equal = define.IndexOf("=");
                        if (equal == -1)
                        {
                            cfg.Clang.Defines.Update(define, "1");
                        }
                        else
                        {
                            cfg.Clang.Defines.Update(define.Substring(0, equal), define.Substring(equal + 1));
                        }
                    }
                }
            };
        }

        private static OptionAction AddClangArgs()
        {
            var argument = new Argument
            {
                ArgumentType = typeof(string),
                Arity = ArgumentArity.OneOrMore,
                Name = "arg",
            };
            argument.SetDefaultValue(Array.Empty<string>());

            return new OptionAction()
            {
                Option = new Option(new[] { "--clang-arg" }, "Arguments verbatimly passed to Clang.")
                {
                    Argument = argument
                },
                Stage = OptionAction.StageEnum.ConfigurationBuilder,
                HandleOption = (opt, cmd, ctx, cfg) =>
                {
                    cfg.Clang.Arguments += string.Join(" ", cmd.ParseResult.ValueForOption<string[]>("clang-arg"));
                }
            };
        }

        internal class CommandLineConsole : IConsole
        {
            public CommandLineConsole()
            {
                m_stringBuilderWriter = new StringBuilderWriter();
                Out = m_stringBuilderWriter;
                Error = m_stringBuilderWriter;
            }

            public IStandardStreamWriter Error { get; protected set; }

            public IStandardStreamWriter Out { get; protected set; }

            public bool IsOutputRedirected { get; protected set; }

            public bool IsErrorRedirected { get; protected set; }

            public bool IsInputRedirected { get; protected set; }

            public override string ToString()
            {
                return m_stringBuilderWriter.ToString();
            }

            internal class StringBuilderWriter : TextWriter, IStandardStreamWriter
            {
                private readonly StringBuilder m_stringBuilder = new StringBuilder();

                public override void Write(char value)
                {
                    m_stringBuilder.Append(value);
                }

                public override Encoding Encoding { get; } = Encoding.Unicode;

                public override string ToString()
                {
                    return m_stringBuilder.ToString();
                }
            }

            private StringBuilderWriter m_stringBuilderWriter;
        }
    }
}
