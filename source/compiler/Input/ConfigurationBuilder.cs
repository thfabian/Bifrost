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
using Bifrost.Compiler.Core;
using YamlDotNet.Serialization;
using YamlDotNet.Core;

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
            Configuration config = null;

            if (!File.Exists(file))
            {
                throw new Exception($"no such file or directory: '{file}'");
            }

            if (Path.GetExtension(file) == ".yaml")
            {
                config = ParseYaml(file);
            }
            else if (Path.GetExtension(file) == ".yaml")
            {
                config = ParseJson(file);
            }
            else
            {
                throw new Exception($"invalid input file: JSON or YAML file is required: '{file}'");
            }

            return config;
        }

        private Configuration ParseYaml(string file, string content = null)
        {
            if (content == null)
            {
                Logger.Debug($"Parsing YAML file: '{file}'");
                content = File.ReadAllText(file);
            }

            var deserializer = new DeserializerBuilder().Build();
            var serializer = new SerializerBuilder().JsonCompatible().Build();

            object yamlObject = null;
            try
            {
                yamlObject = deserializer.Deserialize(new StringReader(content));
            }
            catch (YamlException e)
            {
                var msg = "Failed to parse YAML: " + e.Message.Substring(e.Message.IndexOf("):") + 3);
                var range = new SourceRange(new SourceLocation(file, e.Start.Line, e.Start.Column), new SourceLocation(file, e.End.Line, e.End.Column));
                throw new CompilerError(msg, range);
            }
            return ParseJson(file, serializer.Serialize(yamlObject));
        }

        private Configuration ParseJson(string file, string content = null)
        {
            if (content == null)
            {
                content = File.ReadAllText(file);
            }

            return new Configuration();
        }
    }
}
