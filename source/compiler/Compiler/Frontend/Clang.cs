using System;
using System.Collections.Generic;
using System.Text;
using System.Linq;
using System.IO;
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
        public void Visit(Configuration config, BIR.BIR bir)
        {
        }

        /// <inheritDoc />
        public BIR.BIR Produce(Configuration config)
        {
            using (var section = CreateSection("Invoking Clang"))
            {
                var clangArgs = new List<string>()
                {
                    "-std=c++11",
                    "-xc++",
                    "-Wno-pragma-once-outside-header"
                };

                clangArgs.AddRange(config.Clang.Includes.Select(x => $"-I\"{x}\""));
                clangArgs.AddRange(config.Clang.Defines.Select((x, y) => $"-D{x}=\"{y}\""));
                clangArgs.AddRange(config.Clang.Arguments.Split(" "));

                var tuFlags = CXTranslationUnit_Flags.CXTranslationUnit_SkipFunctionBodies;

                var path = @"C:\Users\fabian\Desktop\Bifrost\source\compiler\Compiler\Test\input.h";

                var index = CXIndex.Create();
                var tuError = CXTranslationUnit.TryParse(index, path, clangArgs.ToArray(), Array.Empty<CXUnsavedFile>(), tuFlags, out CXTranslationUnit tu);
                using (var parser = new ClangParser(Context, tu))
                {

                }
            }
            return null;
        }

        internal class ClangParser : CompilerObject, IDisposable
        {
            private readonly CXTranslationUnit TU;

            public ClangParser(CompilerContext ctx, CXTranslationUnit tu) : base(ctx)
            {
               TU = tu;
            }

            public void Dispose()
            {
                EmitDiagnostics();
            }

            /// <summary>
            /// Emit diagnostics of the translation unit
            /// </summary>
            private void EmitDiagnostics()
            {
                for (uint i = 0; i < TU.NumDiagnostics; i++)
                {
                    var diag = TU.GetDiagnostic(i);
                    if (diag.Severity < CXDiagnosticSeverity.CXDiagnostic_Error)
                    {
                        continue;
                    }

                    var range = new SourceRange(ExtractLocation(diag.Location));
                    if (diag.NumRanges > 0)
                    {
                        range = ExtractRange(diag.GetRange(0));
                    }

                    Diagnostics.Error($"clang error: {diag.Spelling}", range);
                }
            }

            private static SourceLocation ExtractLocation(CXSourceLocation loc)
            {
                loc.GetFileLocation(out CXFile cxfile, out uint line, out uint column, out uint offset);
                return new SourceLocation(cxfile.ToString(), (int)line, (int)column);
            }

            private static SourceRange ExtractRange(CXSourceRange range)
            {
                return new SourceRange(ExtractLocation(range.Start), ExtractLocation(range.End));
            }
        }
    }
}
