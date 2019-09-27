﻿//   ____  _  __               _
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

namespace Bifrost.Compiler.Input
{
    public enum HookTypeEnum
    {
        Class,
        Function
    }

    public enum LanguageEnum
    {
        C99,
        Cpp11,
        Cpp14,
        Cpp17,
    }

    public enum ImportanceEnum
    {
        Required,
        Optional
    }

    public class Configuration
    {

        [Comment("Revision of the configuration.")]
        public System.Version Version { set; get; } = new System.Version(0, 1, 1);

        public class ClangT
        {
            [Comment("Extra arguments passed to Clang.")]
            public string Arguments { get; set; } = "";

            [Comment("Directories added to include search path.")]
            public List<string> Includes { get; set; } = new List<string>();

            [Comment("Used the following macros during compilation.")]
            public Dictionary<string, string> Defines { get; set; } = new Dictionary<string, string>();

            [Comment("Language to compile in.")]
            public LanguageEnum Language { get; set; } = LanguageEnum.Cpp17;
        };

        [Comment("Clang specific options.")]
        public ClangT Clang { set; get; } = new ClangT();

        public class HookT
        {
            public class DescriptionT
            {
                [Comment("Name of the function or class method to hook. Note that the full qualified name needs to be provided (including namespaces).\n" +
                         "For example, 'ID3D12GraphicsCommandList::Close' where 'ID3D12GraphicsCommandList' is the class and 'Close' the method to hook.\n" +
                         "You may use wild cards (*) to generate multiple hooks at once. For example, 'ID3D12GraphicsCommandList::*' will hook all methods \n" +
                         "of the class 'ID3D12GraphicsCommandList'. If the evaluation of the name results in multiple hooks, the field 'identifier' is ignored.",
                        ImportanceEnum.Required)]
                public string Name { get; set; } = "";

                [Comment("The type of hook.", ImportanceEnum.Required)]
                public HookTypeEnum Type { get; set; }

                [Comment("Identifier of the hook, needs to be a valid C/C++ enum value. By default any '::' will be transformed to '_' \n" +
                         "(e.g ID3D12GraphicsCommandList::Close -> ID3D12GraphicsCommandList_Close).")]
                public string Identifier { get; set; } = "";

                [Comment("List of C/C++ input files which need to be included in oder to obtain the declaration of this hook (e.g \"d3d12.h\")", ImportanceEnum.Required)]
                public List<string> Input { get; set; } = new List<string>();
            }

            [Comment("Individual descriptions.")]
            public List<DescriptionT> Descriptions { get; set; } = new List<DescriptionT>();
        }

        [Comment("Hook description.")]
        public HookT Hook { set; get; } = new HookT();

        public class PluginT
        {
            [Comment("Name of the generated Plugin, needs to be a valid C++ class name.", ImportanceEnum.Required)]
            public string Name { get; set; } = "";

            [Comment("Namespace of the generated plugin will be placed in.", ImportanceEnum.Required)]
            public string Namespace { get; set; } = "";
        }

        [Comment("Options of the generated Bifrost plugin.")]
        public PluginT Plugin { set; get; } = new PluginT();
    };

    internal class CommentAttribute : Attribute
    {
        public CommentAttribute(string comment, ImportanceEnum importance = ImportanceEnum.Optional)
        {
            Comment = comment;
            Importance = importance;
        }

        public ImportanceEnum Importance { get; }

        public string Comment { get; }
    }
}