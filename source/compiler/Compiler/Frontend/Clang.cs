using System;
using System.Collections.Generic;
using System.Text;
using System.Linq;
using Bifrost.Compiler.BIR;
using Bifrost.Compiler.Input;
using Bifrost.Compiler.Core;
using ClangSharp.Interop;

namespace Bifrost.Compiler.Frontend
{
    /// <summary>
    /// Clang based BIR producer
    /// </summary>
    public class Clang : CompilerObject, IFrontendAction
    {
        public Clang(CompilerContext ctx) : base(ctx) { }

        /// <inheritDoc />
        public BIR.BIR Produce(Configuration config)
        {
            var clangArgs = new List<string>()
            {
                "-std=c++11",                     // The input files should be compiled for C++11
                "-xc++",                          // The input files are C++
                "-Wno-pragma-once-outside-header" // We are processing files which may be header files
            };

            clangArgs.AddRange(config.Clang.Includes.Select(x => $"-I\"{x}\""));
            clangArgs.AddRange(config.Clang.Defines.Select((x, y) => $"-D{x}=\"{y}\""));
            clangArgs.AddRange(config.Clang.Arguments.Split(" "));

            var translationFlags = CXTranslationUnit_Flags.CXTranslationUnit_SkipFunctionBodies;

            var index = CXIndex.Create();
            var translationUnitError = CXTranslationUnit.TryParse(index, @"C:\Users\fabian\Desktop\Bifrost\source\compiler\Compiler\Test\input.h", clangArgs.ToArray(), Array.Empty<CXUnsavedFile>(), translationFlags, out CXTranslationUnit handle);
            


            return null;
        }

        /// <inheritDoc />
        public void Visit(Configuration config, BIR.BIR bir)
        {
        }
    }
}
