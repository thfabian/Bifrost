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
using System.Text;

namespace Bifrost.Compiler.BIR
{
    /// <summary>
    /// Bifrost Intermediate Representation 
    /// </summary>
    public class BIR
    {
        public enum HookTypeEnum
        {
            CFunction,
            MethodFunction,
            VTable
        }

        /// <summary>
        /// Description of an individual hook
        /// </summary>
        public class Hook
        {
            /// <summary>
            /// Type of hook
            /// </summary>
            public HookTypeEnum HookType;

            /// <summary>
            /// Unique identifier of this hook (C++ enum value compatible)
            /// </summary>
            public string Identifier;

            /// <summary>
            /// Return type of the method/function
            /// </summary>
            public string ReturnType;

            /// <summary>
            /// Module (DLL) to load to obtain this function
            /// </summary>
            public string Module;

            /// <summary>
            /// Input headers required for the declaration 
            /// </summary>
            public List<string> Inputs;

            #region CFunction specific
            /// <summary>
            /// Name of the function
            /// </summary>
            public string CFunctionName;
            #endregion

            #region VTable specific
            /// <summary>
            /// Type of the "this" pointer (only relevant for VTable hooks)
            /// </summary>
            public string VTableThisType;
            #endregion

            public class Parameter
            {
                /// <summary>
                /// Name of the parameter argument
                /// </summary>
                public string Name;

                /// <summary>
                /// Full CV-qualified type
                /// </summary>
                public string Type;
            }
            public List<Parameter> Parameters = new List<Parameter>();
        }
        public List<Hook> Hooks = new List<Hook>();
    }
}
