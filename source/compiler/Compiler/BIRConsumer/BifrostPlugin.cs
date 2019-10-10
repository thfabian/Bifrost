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
                ["BIFROST_NAMESPACE"] = string.IsNullOrEmpty(config.Plugin.Namespace) ? config.Plugin.Name.ToLower() : config.Plugin.Namespace
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
            var macrosToParse = new List<string>()
            {
                "BIFROST_NAMESPACE_BEGIN",
                "BIFROST_NAMESPACE_END",
            };
            return PreprocessorUtils.ExpandMacros(plugin, macrosToParse, predefinedMacros);
        }
    }
}
