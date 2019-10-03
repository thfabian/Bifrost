using System;
using System.Collections.Generic;
using System.Text;
using System.Linq;
using System.IO;
using System.Text.RegularExpressions;
using Bifrost.Compiler.BIR;
using Bifrost.Compiler.Input;
using Bifrost.Compiler.Core;

namespace Bifrost.Compiler.BIRConsumer
{
    /// <summary>
    /// Generate a Bifrost Plugin
    /// </summary>
    public class BifrostPlugin : CompilerObject, IBIRConsumer
    {
        public BifrostPlugin(CompilerContext ctx) : base(ctx)
        {
        }

        /// <inheritDoc />
        public bool Consume(Configuration config, BIR.BIR bir)
        {
            using (var section = CreateSection("Generating Bifrost C++ plugin"))
            {
                var plugin = AssemblyInfo.GetEmbeddedResource("plugin_main.h");

                // Create the macros
                var macros = CreateMacros(config, bir);

                // Expand the includes
                plugin = ExpandIncludes(plugin);

                // Expand selected macros
                plugin = ExpandMacros(plugin, macros);

                File.WriteAllText(@"C:\Users\fabian\Desktop\Bifrost\build-vs2019\plugin.cpp", plugin);
                section.Done();
            }
            return true;
        }

        /// <summary>
        /// Create the necessary macros
        /// </summary>
        private Dictionary<string, string> CreateMacros(Configuration config, BIR.BIR bir)
        {
            return new Dictionary<string, string>()
            {
                ["BIFROST_NAMESPACE"] = config.Plugin.Namespace
            };
        }

        /// <summary>
        /// Expand certain includes
        /// </summary>
        private string ExpandIncludes(string plugin)
        {
            var pluginH = AssemblyInfo.GetEmbeddedResource("plugin.h");
            pluginH = pluginH.Substring(pluginH.IndexOf("#pragma once") + "#pragma once".Length);
            plugin = plugin.Replace("#include \"bifrost/api/plugin.h\"", pluginH);

            var pluginFwdH = AssemblyInfo.GetEmbeddedResource("plugin_fwd.h");
            plugin = plugin.Replace("#include \"bifrost/template/plugin_fwd.h\"", pluginFwdH);
            return plugin;
        }

        /// <summary>
        /// Expand certain macros directly
        /// </summary>
        private string ExpandMacros(string plugin, Dictionary<string, string> predefinedMacros)
        {
            var lines = new List<string>();
            var regex = new Regex(@"#define (\S*) (.*)");

            var expandMacros = new List<string>()
            {
                "BIFROST_NAMESPACE_BEGIN",
                "BIFROST_NAMESPACE_END",
            };
            var macros = new Dictionary<string, string>(predefinedMacros);

            foreach (var line in plugin.Split("\n"))
            {
                // Expand value
                bool insideString = false;
                int lastIndex = 0;
                var tokens = new List<string>();
                for(var idx = 0; idx < line.Length; ++idx)
                {
                    var c = line[idx];

                    // We can't expand tokens inside strings
                    if(c == '\"')
                    {
                        insideString = !insideString;
                    }

                    if(!insideString && c == ' ')
                    {
                        tokens.Add(line.Substring(lastIndex, idx - lastIndex));
                        lastIndex = idx++;

                        // Consume as many whitespaces as we can
                        for (; idx < line.Length; ++idx)
                        {
                            var wc = line[idx];
                            if(wc != ' ' || idx == (line.Length - 1))
                            {
                                tokens.Add(line.Substring(lastIndex, idx - lastIndex));
                                lastIndex = idx++;
                                break;
                            }
                        }
                    }
                }

                var newLineBuilder = new StringBuilder();
                foreach(var token in tokens)
                {
                    foreach (var (key, value) in macros)
                    {
                        if(token == key)
                        {
                            newLineBuilder.Append(value);
                        }
                        else
                        {
                            newLineBuilder.Append(token);
                        }
                    }
                }
                var newLine = newLineBuilder.ToString();

                // Extract value
                bool skip = false;
                foreach (var macro in expandMacros)
                {
                    var match = regex.Match(newLine);
                    if (match.Success)
                    {
                        var key = match.Groups[1].Value;
                        var value = match.Groups[2].Value.Replace("\r", "");
                        if (key == macro)
                        {
                            Logger.Debug($"Found value of macro '{key}': '{value}'");
                            macros.Add(key, value);
                            skip = true;
                        }
                    }
                }

                if (!skip)
                {
                    lines.Add(newLine);
                }
            }

            return string.Join("\n", lines);
        }
    }
}
