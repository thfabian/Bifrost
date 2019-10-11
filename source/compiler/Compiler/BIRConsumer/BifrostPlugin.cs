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
using System.IO;
using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;
using System.Linq;
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
                var pluginH = GetPluginH();
                var pluginCpp = GetPluginCpp();

                // Create the macros
                var macros = CreateMacros(config, bir);

                // Expand the includes
                pluginH = ExpandIncludes(pluginH);

                // Expand selected macros
                pluginH = ExpandMacros(pluginH, macros);
                pluginCpp = ExpandMacros(pluginCpp, macros);

                // Write plugin to disk and format it 
                WriteOutputs(config, pluginH, pluginCpp, macros);
                section.Done();
            }
            return true;
        }

        /// <summary>
        /// Write final outputs to disk
        /// </summary>
        private void WriteOutputs(Configuration config, string pluginHContent, string pluginCppContent, Dictionary<string, string> macros)
        {
            Utils.IO.CreateDirectory(config.General.OutputPath);
            var clangFormat = new ClangFormat(Context);

            // Write header
            var pluginH = Path.Combine(config.General.OutputPath, macros["BIFROST_PLUGIN_H_FILE"]);
            File.WriteAllText(pluginH, pluginHContent);
            Logger.Debug($"Writting plugin header file to: \"{pluginH}\"");
            clangFormat.Format(pluginH);

            // Write cpp
            var pluginCpp = Path.Combine(config.General.OutputPath, macros["BIFROST_PLUGIN_CPP_FILE"]);
            if (!File.Exists(pluginCpp))
            {
                File.WriteAllText(pluginCpp, pluginCppContent);
                Logger.Debug($"Writting plugin source file to: \"{pluginCpp}\"");
                clangFormat.Format(pluginCpp);
            }
        }

        /// <summary>
        /// Create the necessary macros
        /// </summary>
        private Dictionary<string, string> CreateMacros(Configuration config, BIR.BIR bir)
        {
            var pluginName = IO.MakeValidIdentifier(config.Plugin.Name);
            var pluginNamespace = IO.MakeValidIdentifier(string.IsNullOrEmpty(config.Plugin.Namespace) ? pluginName.ToLower() : config.Plugin.Namespace);
            var pluginCppFile = $"{pluginName}.cpp";
            var pluginHFile = $"{pluginName}.h";

            var pluginIdentifier = new List<string>();
            var pluginStringToIdentifier = new List<string>();
            var pluginIdentifierToFunctionName = new List<string>();
            var pluginIdentifierToModule = new List<string>();

            var includes = new HashSet<string>();

            var module = new List<string>();
            var moduleToString = new List<string>();
            var hookModuleToModule = new Dictionary<string, string>();

            var dslDefines = new List<(string, string)>();

            foreach (var hook in bir.Hooks)
            {
                // Identifiers
                pluginIdentifier.Add($"{hook.Identifier}");
                pluginStringToIdentifier.Add($"{{\"{hook.Identifier}\", Plugin::Identifier::{hook.Identifier}}}");

                switch (hook.HookType)
                {
                    case BIR.BIR.HookTypeEnum.CFunction:
                        pluginIdentifierToFunctionName.Add($"\"{hook.CFunctionName}\"");
                        break;
                    case BIR.BIR.HookTypeEnum.MethodFunction:
                        pluginIdentifierToFunctionName.Add($"\"\"");
                        break;
                    case BIR.BIR.HookTypeEnum.VTable:
                        pluginIdentifierToFunctionName.Add($"\"\"");
                        break;
                }

                // Includes
                foreach (var include in hook.Inputs)
                {
                    includes.Add($"#include <{include}>");
                }

                // Modules
                var moduleIdentifier = "";
                if (hookModuleToModule.ContainsKey(hook.Module))
                {
                    moduleIdentifier = hookModuleToModule[hook.Module];
                }
                else
                {
                    moduleIdentifier = IO.MakeValidIdentifier(hook.Module);
                    hookModuleToModule.Add(hook.Module, moduleIdentifier);

                    module.Add(moduleIdentifier);
                    moduleToString.Add(hook.Module);
                }
                pluginIdentifierToModule.Add($"Module::{moduleIdentifier}");

                // DSL
                var postfix = $"{pluginNamespace}__{hook.Identifier}";
                dslDefines.Add((null, $"\n// {hook.Identifier}"));
                dslDefines.Add(($"_bf_func_decl_ret_{postfix}", hook.ReturnType));
                dslDefines.Add(($"_bf_func_decl_args_{postfix}", string.Join(", ", hook.Parameters.Select(p => p.Type + " " + p.Name))));

                dslDefines.Add(($"_bf_func_{postfix}", $"(({hook.ReturnType} (*)(" + string.Join(", ", hook.Parameters.Select(p => p.Type)) +
                                $"))::{pluginNamespace}::Plugin::Get().GetHook<::{pluginNamespace}::Plugin::Identifier::{hook.Identifier}>()->GetOriginal())"));
                dslDefines.Add(($"_bf_args_{postfix}", string.Join(", ", hook.Parameters.Select(p => p.Name))));

                for (int i = 0; i < hook.Parameters.Count; ++i)
                {
                    dslDefines.Add(($"_bf_arg_{i + 1}_{postfix}", hook.Parameters[i].Name));
                }
            }

            var dslDef = string.Join("\n", dslDefines.Select(s => s.Item1 == null ? s.Item2 : $"#define {s.Item1} {s.Item2}"));

            return new Dictionary<string, string>()
            {
                ["BIFROST_PLUGIN_H_FILE"] = pluginHFile,
                ["BIFROST_PLUGIN_CPP_FILE"] = pluginCppFile,
                ["BIFROST_PLUGIN_NAME"] = pluginName,
                ["BIFROST_PLUGIN_BASE"] = $"::{pluginNamespace}::Plugin",
                ["BIFROST_PLUGIN_INCLUDE"] = $"#include \"{pluginHFile}\"",

                ["BIFROST_NAMESPACE"] = pluginNamespace,
                ["BIFROST_PLUGIN_IDENTIFIER"] = string.Join(",", pluginIdentifier) + ",",
                ["BIFROST_PLUGIN_IDENTIFIER_TO_STRING"] = "\"" + string.Join("\",\"", pluginIdentifier) + "\",",
                ["BIFROST_PLUGIN_STRING_TO_IDENTIFIER"] = string.Join(",", pluginStringToIdentifier),
                ["BIFROST_PLUGIN_IDENTIFIER_TO_FUNCTION_NAME"] = string.Join(",", pluginIdentifierToFunctionName) + ",",

                ["BIFROST_PLUGIN_MODULE"] = string.Join(",", module) + ",",
                ["BIFROST_PLUGIN_MODULE_TO_STRING"] = "L\"" + string.Join("\",L\"", moduleToString) + "\",",
                ["BIFROST_PLUGIN_IDENTIFIER_TO_MODULE"] = string.Join(",", pluginIdentifierToModule) + ",",
                
                ["BIFROST_PLUGIN_DSL_DEF"] = dslDef,

                ["BIFROST_PLUGIN_INCLUDES"] = string.Join("\n", includes)
            };
        }

        /// <summary>
        /// Expand certain includes
        /// </summary>
        private string ExpandIncludes(string content)
        {
            var pluginH = AssemblyInfo.GetEmbeddedResource("plugin.h");
            pluginH = pluginH.Substring(pluginH.IndexOf("#pragma once") + "#pragma once".Length);
            content = content.Replace("#include \"bifrost/api/plugin.h\"", pluginH);

            var pluginFwdH = AssemblyInfo.GetEmbeddedResource("plugin_fwd.h");
            content = content.Replace("#include \"bifrost/template/plugin_fwd.h\"", pluginFwdH);
            return content;
        }

        /// <summary>
        /// Expand certain macros directly
        /// </summary>
        private string ExpandMacros(string content, Dictionary<string, string> predefinedMacros)
        {
            var macrosToParse = new List<string>()
            {
                "BIFROST_NAMESPACE_BEGIN",
                "BIFROST_NAMESPACE_END",
            };
            return PreprocessorUtils.ExpandMacros(content, macrosToParse, predefinedMacros);
        }

        /// <summary>
        /// Get the header file template
        /// </summary>
        private string GetPluginH()
        {
            var pluginH = AssemblyInfo.GetEmbeddedResource("plugin_main.h");
            pluginH = AddPreamble(pluginH, false);
            return pluginH;
        }

        /// <summary>
        /// Get the C++ file template
        /// </summary>
        private string GetPluginCpp()
        {
            var pluginCpp = AssemblyInfo.GetEmbeddedResource("plugin_main.cpp");
            pluginCpp = AddPreamble(pluginCpp, true);
            return pluginCpp;
        }

        /// <summary>
        /// Add a preamble to the <paramref name="content"/>
        /// </summary>
        private string AddPreamble(string content, bool editable)
        {
            var preamble = "";
            preamble += $"//\n";
            if (!editable)
            {
                preamble += $"// ================================================================\n";
                preamble += $"//           WARNING --- DO NOT EDIT THIS FILE --- WARNING\n";
                preamble += $"// ================================================================\n";
                preamble += $"//\n";
            }
            preamble += $"// Generated by bfc ({Core.AssemblyInfo.Version}) - {DateTime.Now}\n";
            preamble += $"//\n";
            return preamble + content;
        }
    }
}
