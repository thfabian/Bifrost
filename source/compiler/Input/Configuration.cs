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

namespace Bifrost.Compiler.Input
{
    public class Configuration
    {
        [Comment("Revision of the Configuration")]
        public System.Version Version { set; get; } = new System.Version(0, 1, 1);

        public class ClangT
        {
            [Comment("Extra arguments passed to Clang")]
            public string Arguments { get; set; } = "";

            [Comment("Directories added to include search path")]
            public List<string> Includes { get; set; } = new List<string>();

            [Comment("Define macros")]
            public Dictionary<string, string> Defines { get; set; } = new Dictionary<string, string>();
        };

        [Comment("Clang specific options")]
        public ClangT Clang { set; get; } = new ClangT();
    };

    internal class CommentAttribute : Attribute
    {
        public CommentAttribute(string comment)
        {
            Comment = comment;
        }

        public string Comment { get; }
    }
}