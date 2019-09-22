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
using System.IO;
using System.Runtime.Serialization;
using YamlDotNet.Core;
using YamlDotNet.Serialization;
using YamlDotNet.Serialization.NamingConventions;
using Bifrost.Compiler.Core;

namespace Bifrost.Compiler.Input
{
    /// <summary>
    /// Build the input configuration which configures the compiler passes
    /// </summary>
    public class ConfigurationBuilder : CompilerObject
    {
        public ConfigurationBuilder(CompilerContext ctx) : base(ctx) { }

        /// <summary>
        /// Build a configuration given the input files <paramref name="files"/> and the parsed command-line <paramref name="cmd"/>
        /// </summary>
        public Configuration Build(CommandLine cmd, string file)
        {
            Configuration config = null;
            using (var section = CreateSection("Building input configuration"))
            {
                // Read configuration from file
                config = Parse(file);

                // Update the configuration from the command-line options
                cmd.UpdateConfiguration(Context, config);

                section.Done();
            }
            return config;
        }

        private Configuration Parse(string file)
        {
            if (!File.Exists(file))
            {
                throw new Exception($"no such file or directory: '{file}'");
            }

            Logger.Debug($"Parsing YAML file: '{file}'");
            var yaml = File.ReadAllText(file);

            try
            {
                var deserializer = new DeserializerBuilder()
                    .WithNamingConvention(new CamelCaseNamingConvention())
                    .Build();

                return deserializer.Deserialize<Configuration>(yaml);
            }
            catch (YamlException e)
            {
                var msg = "invalid configuration: ";
                if (e.InnerException != null && e.InnerException.GetType() == typeof(SerializationException))
                {
                    var innerMsg = e.InnerException.Message.Replace("Bifrost.Compiler.Input.Configuration", "Configuration");
                    msg += char.ToLower(innerMsg[0]) + innerMsg.Substring(1);
                }
                else
                {
                    msg += e.Message.Substring(e.Message.IndexOf("):") + 3);
                }

                var range = new SourceRange(new SourceLocation(file, e.Start.Line, e.Start.Column), new SourceLocation(file, e.End.Line, e.End.Column));
                throw new CompilerError(msg, range);
            }
        }
    }
}
