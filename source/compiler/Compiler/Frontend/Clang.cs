using System;
using System.Collections.Generic;
using System.Text;
using System.Linq;
using System.IO;
using Bifrost.Compiler.BIR;
using Bifrost.Compiler.Input;
using Bifrost.Compiler.Core;
using ClangSharp;
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
        private TranslationUnit BuildClangAST(Configuration config, CXIndex index)
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
            if (tu == null)
            {
                return null;
            }
            return TranslationUnit.GetOrCreate(tu);
        }

        /// <summary>
        /// Transform the Clang AST to BIR
        /// </summary>
        private BIR.BIR TransformClangASTtoBIR(Configuration config, CXIndex index, TranslationUnit tu)
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
            public readonly TranslationUnit TU;

            /// <summary>
            /// Current CXIndex (pointer to AST)
            /// </summary>
            public CXIndex Index;

            /// <summary>
            /// Diagnostics have been emitted?
            /// </summary>
            private bool m_diagEmitted = false;

            public ClangParser(CompilerContext ctx, CXIndex index, TranslationUnit tu) : base(ctx)
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

                var visitor = new ClangASTVisitor(Context, config);
                return visitor.VisitTU(TU);
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

                for (uint i = 0; i < TU.Handle.NumDiagnostics; i++)
                {
                    using (var diag = TU.Handle.GetDiagnostic(i))
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
                for (uint i = 0; i < TU.Handle.NumDiagnostics; i++)
                {
                    using (var diag = TU.Handle.GetDiagnostic(i))
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

        /// <summary>
        /// Visit the Clang AST
        /// </summary>
        internal class ClangASTVisitor : CompilerObject
        {
            /// <summary>
            /// BIR we are constructing
            /// </summary>

            private BIR.BIR m_bir = new BIR.BIR();

            /// <summary>
            /// Configuration we are currently parsing
            /// </summary>

            private readonly Configuration m_config;

            /// <summary>
            /// Functions we need to hook
            /// </summary>
            private readonly HashSet<string> m_hookFunctions = new HashSet<string>();

            /// <summary>
            /// Hook functions we have visited
            /// </summary>
            private HashSet<string> m_hookFunctionsVisited = new HashSet<string>();

            /// <summary>
            /// Cursors which we already encountered
            /// </summary>
            private HashSet<Cursor> m_visitedCursors = new HashSet<Cursor>();

            public ClangASTVisitor(CompilerContext ctx, Configuration config) : base(ctx)
            {
                m_config = config;
                foreach (var desc in m_config.Hook.Descriptions)
                {
                    if (desc.Type == HookTypeEnum.Function)
                    {
                        m_hookFunctions.Add(desc.Name);
                    }
                }
            }

            /// <summary>
            /// Visit all AST nodes of the translation unit
            /// </summary>
            public BIR.BIR VisitTU(TranslationUnit tu)
            {
                var tuDecl = tu.TranslationUnitDecl;
                m_visitedCursors.Add(tuDecl);

                foreach (var decl in tuDecl.Decls)
                {
                    Visit(decl);
                }

                return m_bir;
            }

            /// <summary>
            /// Main dispatch method
            /// </summary>
            private void Visit(Cursor cursor)
            {
                // The AST can by cyclic
                if (m_visitedCursors.Contains(cursor))
                {
                    return;
                }
                m_visitedCursors.Add(cursor);

                if (cursor is Decl decl)
                {
                    VisitDecl(decl);
                }
            }

            /// <summary>
            /// Visit a declaration cursor
            /// </summary>
            private void VisitDecl(Decl decl)
            {
                if (decl is FunctionDecl funDecl)
                {
                    VisitFunctionDecl(funDecl);
                }
            }

            /// <summary>
            /// Visit a function declaration
            /// </summary>
            private void VisitFunctionDecl(FunctionDecl funDecl)
            {
                if (m_hookFunctions.Contains(funDecl.Spelling))
                {
                    RegisterFunctionHook(funDecl);
                }
            }

            /// <summary>
            /// Register a C function hook
            /// </summary>
            private void RegisterFunctionHook(FunctionDecl decl)
            {
                var name = decl.Name;
                var type = decl.Type;
            }
        }
    }
}
