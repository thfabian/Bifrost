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
            // https://shaharmike.com/cpp/libclang/
            using (var index = CXIndex.Create())
            {
                using (var tu = BuildClangAST(config, index))
                {
                    if (tu != null)
                    {
                        return TransformClangASTtoBIR(config, index, tu);
                    }
                }
            }
            return null;
        }

        /// <summary>
        /// Build the Clang AST 
        /// </summary>
        private CXTranslationUnit BuildClangAST(Configuration config, CXIndex index)
        {
            CXTranslationUnit tu = null;
            using (var section = CreateSection("Building Clang AST"))
            {
                // Assemble the arguments
                var clangArgs = new List<string>()
                {
                    "-Wno-pragma-once-outside-header"
                };

                switch (config.Clang.Language)
                {
                    case LanguageEnum.C99:
                        clangArgs.Add("-xc");
                        clangArgs.Add("-std=c99");
                        break;
                    case LanguageEnum.Cpp11:
                        clangArgs.Add("-xc++");
                        clangArgs.Add("-std=c++11");
                        break;
                    case LanguageEnum.Cpp14:
                        clangArgs.Add("-xc++");
                        clangArgs.Add("-std=c++14");
                        break;
                    case LanguageEnum.Cpp17:
                        clangArgs.Add("-xc++");
                        clangArgs.Add("-std=c++17");
                        break;
                    default:
                        throw new Exception($"unknown language {config.Clang.Language}");
                }

                clangArgs.AddRange(config.Clang.Includes.Select(x => $"-I{IO.Normalize(IO.Shorten(x))}"));
                clangArgs.AddRange(config.Clang.Defines.Select((x, y) => $"-D{x}=\"{y}\""));
                clangArgs.AddRange(config.Clang.Arguments.Split(" "));

                var tuFlags = CXTranslationUnit_Flags.CXTranslationUnit_SkipFunctionBodies;

                // Create the TU source file
                var sourceFiles = new HashSet<string>();
                config.Hook.Descriptions.ForEach(hook => hook.Input.ForEach(source => sourceFiles.Add(source)));

                var tuFilename = Path.GetFileNameWithoutExtension(string.IsNullOrEmpty(config.Plugin.Name) ? Path.GetTempFileName() : config.Plugin.Name) + ".cpp";
                Logger.Debug($"Clang TU filename: {tuFilename}");

                var tuSource = "#include <" + string.Join(">\n#include <", sourceFiles) + ">";

                Logger.Debug($"Clang TU source:\n{tuSource}");
                using (var tuSourceFile = CXUnsavedFile.Create(tuFilename, tuSource))
                {
                    // Invoke clang to get the TU
                    Logger.Debug($"Clang args: {string.Join(" ", clangArgs)}");
                    var tuError = CXTranslationUnit.TryParse(index, tuSourceFile.FilenameString, clangArgs.ToArray(), new CXUnsavedFile[] { tuSourceFile }, tuFlags, out tu);
                    if (tuError != CXErrorCode.CXError_Success)
                    {
                        throw new Exception($"clang error: failed to generate tanslation unit: {tuError}");
                    }
                }
                section.Done();
            }
            return tu;
        }

        /// <summary>
        /// Transform the Clang AST to BIR
        /// </summary>
        private BIR.BIR TransformClangASTtoBIR(Configuration config, CXIndex index, CXTranslationUnit tu)
        {
            BIR.BIR bir = null;
            using (var section = CreateSection("Transforming Clang AST to BIR"))
            {
                using (var parser = new ClangParser(Context, index, tu))
                {
                    bir = parser.ProduceBIR(config);
                }

                if (bir != null)
                {
                    section.Done();
                }
            }
            return bir;
        }

        internal class ClangParser : CompilerObject, IDisposable
        {
            /// <summary>
            /// Reference to the translation unit
            /// </summary>
            public readonly CXTranslationUnit TU;

            /// <summary>
            /// Current CXIndex (pointer to AST)
            /// </summary>
            public CXIndex Index;

            /// <summary>
            /// Diagnostics have been emitted?
            /// </summary>
            private bool m_diagEmitted = false;

            public ClangParser(CompilerContext ctx, CXIndex index, CXTranslationUnit tu) : base(ctx)
            {
                Index = index;
                TU = tu;
            }

            public BIR.BIR ProduceBIR(Configuration config)
            {
                EmitTUErrors();
                if (HasTUErrors())
                {
                    return null;
                }

                TU.Cursor.VisitChildren(() = >)

                return null;
            }

            public void Dispose()
            {
                EmitTUErrors();
            }

            /// <summary>
            /// Emit diagnostics of the translation unit
            /// </summary>
            private void EmitTUErrors()
            {
                if (m_diagEmitted)
                {
                    return;
                }

                for (uint i = 0; i < TU.NumDiagnostics; i++)
                {
                    using (var diag = TU.GetDiagnostic(i))
                    {
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
                m_diagEmitted = true;
            }

            /// <summary>
            /// Do we have errors in the AST?
            /// </summary>
            private bool HasTUErrors()
            {
                for (uint i = 0; i < TU.NumDiagnostics; i++)
                {
                    using (var diag = TU.GetDiagnostic(i))
                    {
                        if (diag.Severity >= CXDiagnosticSeverity.CXDiagnostic_Error)
                        {
                            return true;
                        }
                    }
                }
                return false;
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
