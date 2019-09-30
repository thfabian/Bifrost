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
            Function,
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
            /// Type of the "this" pointer (only relevant for VTable hooks)
            /// </summary>
            public string ThisType;

            /// <summary>
            /// Calling convention of the function
            /// </summary>
            public string CallingConvention;

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
            public List<Parameter> Parameters;
        }
        public List<Hook> Hooks;
    }
}
