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
using System.Linq;
using System.IO;
using Bifrost.Compiler.BIR;
using Bifrost.Compiler.Input;
using Bifrost.Compiler.Core;
using ClangSharp;
using ClangSharp.Interop;

namespace Bifrost.Compiler.BIRProducer
{
    /// <summary>
    /// Clang based BIR producer
    /// </summary>
    public class Clang : CompilerObject, IBIRProducer
    {
        public Clang(CompilerContext ctx) : base(ctx) { }

        /// <inheritDoc />
        public void Visit(Configuration config, BIR.BIR bir)
        {
        }

        /// <inheritDoc />
        public BIR.BIR Produce(Configuration config)
        {
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

                clangArgs.AddRange(GetWinSDKDIncludeArgs(config));
                clangArgs.AddRange(GetCxxSTLIncludeArgs(config));

                clangArgs.AddRange(config.Clang.Includes.Select(i => MakeIncludeArg(i)));
                clangArgs.AddRange(config.Clang.Defines.Select((m, v) => $"-D{m}=\"{v}\""));
                clangArgs.AddRange(config.Clang.Arguments.Split(" "));

                var tuFlags = CXTranslationUnit_Flags.CXTranslationUnit_SkipFunctionBodies;

                // Create the TU source file
                var sourceFiles = new HashSet<string>();
                config.Hook.Descriptions.Values.ToList().ForEach(hook => hook.Input.ForEach(source => sourceFiles.Add(source)));

                var tuFilename = Path.GetFileNameWithoutExtension(string.IsNullOrEmpty(config.Plugin.Name) ? Path.GetTempFileName() : config.Plugin.Name) + ".cpp";
                Logger.Debug($"Clang TU filename: {tuFilename}");

                var tuSource = "#include <" + string.Join(">\n#include <", sourceFiles) + ">";

                Logger.Debug($"Clang TU source:\n  {tuSource.Replace("\n", "\n  ")}");
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
        /// Get the include directories which are needed to compile Win SDK headers.
        /// </summary>
        private IEnumerable<string> GetWinSDKDIncludeArgs(Configuration config)
        {
            string winSDKDir = null;

            if (config.Clang.WinSDK == Configuration.ClangT.None)
            {
                Logger.Debug("Disabling Win SDK include directories.");
                return Array.Empty<string>();
            }
            else if (!string.IsNullOrEmpty(config.Clang.WinSDK) && config.Clang.WinSDK != Configuration.ClangT.Latest)
            {
                winSDKDir = config.Clang.WinSDK;
            }
            else
            {
                Logger.Debug($"Auto detecting Win SDK include directory ...");
                var includeDir = @"C:/Program Files (x86)/Windows Kits/10/Include";

                // Get latest SDK
                int winSDKLatest = -1;
                string winSDKLatestString = "";
                foreach (var dir in IO.GetDirectories(includeDir, "*"))
                {
                    var versionStr = Path.GetFileName(dir).Replace(".", "");
                    if (int.TryParse(versionStr, out int version))
                    {
                        if (version > winSDKLatest)
                        {
                            winSDKLatest = version;
                            winSDKLatestString = Path.GetFileName(dir);
                        }
                    }
                }

                var sdkDir = Path.Combine(includeDir, winSDKLatestString);
                if (Directory.Exists(sdkDir))
                {
                    winSDKDir = sdkDir;
                }
            }

            if (!string.IsNullOrEmpty(winSDKDir))
            {
                Logger.Debug($"Using Win SDK include directory: {IO.Normalize(winSDKDir)}");
                var winSDKDirs = new List<string>();
                foreach (var subDir in new[] { "um", "ucrt", "winrt", "shared", "cppwinrt/winrt" })
                {
                    winSDKDirs.Add(MakeIncludeArg(Path.Combine(winSDKDir, subDir)));
                }
                return winSDKDirs;
            }
            else
            {
                throw new Exception($"Win SDK include directories are not available, set 'clang.winSDK' to '{Configuration.ClangT.None}' to disable Win SDK directory detection.");
            }
        }

        /// <summary>
        /// Get the include directories which are needed to compile C++ STL headers.
        /// </summary>
        private IEnumerable<string> GetCxxSTLIncludeArgs(Configuration config)
        {
            if (config.Clang.CxxSTL == Configuration.ClangT.None)
            {
                Logger.Debug("Disabling C++ STL include directory.");
                return Array.Empty<string>();
            }
            else if (!string.IsNullOrEmpty(config.Clang.CxxSTL) && config.Clang.CxxSTL != Configuration.ClangT.Latest)
            {
                Logger.Debug($"Using C++ STL include directory: {config.Clang.CxxSTL}");
                return new[] { MakeIncludeArg(config.Clang.CxxSTL) };
            }
            else
            {
                Logger.Debug($"Auto detecting C++ STL include directory ...");

                var vsDir = @"C:/Program Files (x86)/Microsoft Visual Studio";
                var vsToolsDir = @"VC/Tools/MSVC";

                foreach (var vsKind in new string[] { "Professional", "Enterprise", "Community" })
                {
                    // Get latest VS installation (e.g 2019)
                    int vsLatest = -1;
                    foreach (var dir in IO.GetDirectories(vsDir, "*"))
                    {
                        var versionString = Path.GetFileName(dir);
                        if (int.TryParse(versionString, out int version))
                        {
                            vsLatest = Math.Max(vsLatest, version);
                        }
                    }

                    var curVsToolsDir = Path.Combine(vsDir, vsLatest.ToString(), vsKind, vsToolsDir);
                    if (!Directory.Exists(curVsToolsDir))
                    {
                        continue;
                    }

                    // Get latest MSVC version
                    int msvcLatest = -1;
                    string msvcLatestString = "-1";
                    foreach (var dir in IO.GetDirectories(Path.Combine(vsDir, vsLatest.ToString(), vsKind, vsToolsDir), "*"))
                    {
                        var versionStr = Path.GetFileName(dir).Replace(".", "");
                        if (int.TryParse(versionStr, out int version))
                        {
                            if (version > msvcLatest)
                            {
                                msvcLatest = version;
                                msvcLatestString = Path.GetFileName(dir);
                            }
                        }
                    }
                    if (msvcLatest == -1)
                    {
                        continue;
                    }

                    var includeDir = IO.Normalize(Path.Combine(vsDir, vsLatest.ToString(), vsKind, vsToolsDir, msvcLatestString, "include"));
                    if (Directory.Exists(includeDir))
                    {
                        Logger.Debug($"Using C++ STL include directory: {IO.Normalize(includeDir)}");
                        return new[] { MakeIncludeArg(includeDir) };
                    }
                }
                throw new Exception($"C++ STL include directories are not available, set 'clang.cxxSTL' to '{Configuration.ClangT.None}' to disable C++ STL include directory detection.");
            }
        }

        /// <summary>
        /// Generate include (-I)
        /// </summary>
        private string MakeIncludeArg(string include)
        {
            return $"-I{IO.Normalize(IO.Shorten(include))}";
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
                if (EmitTUErrors())
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
            /// Emit diagnostics of the translation unit and return true if an error has been emitted
            /// </summary>
            private bool EmitTUErrors()
            {
                if (m_diagEmitted)
                {
                    return false;
                }

                bool hasError = false;
                var numDiag = TU.Handle.NumDiagnostics;
                for (uint i = 0; i < numDiag; i++)
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
                        hasError = true;
                    }
                }
                m_diagEmitted = true;
                return hasError;
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
            /// Did we had an error?
            /// </summary>
            private bool m_hasError = false;

            /// <summary>
            /// Configuration we are currently parsing
            /// </summary>

            private readonly Configuration m_config;

            /// <summary>
            /// Input files we are considering
            /// </summary>

            private readonly HashSet<string> m_relevantInputFilenames = new HashSet<string>();

            /// <summary>
            /// List of all files which took part in the AST generation
            /// </summary>

            private HashSet<string> m_inputFiles = new HashSet<string>();

            /// <summary>
            /// Cursors which we already encountered
            /// </summary>
            private HashSet<Cursor> m_visitedCursors = new HashSet<Cursor>();

            /// <summary>
            /// Data used during traversal
            /// </summary>
            public class TraversalData
            {
                /// <summary>
                /// Get the namespace
                /// </summary>
                public string GetNamespaces()
                {
                    if (m_namespaces.Count == 0) return "";
                    return string.Join("::", m_namespaces);
                }

                /// <summary>
                /// Get classes
                /// </summary>
                public string GetClasses()
                {
                    var ns = GetNamespaces();
                    var classes = string.Join("::", m_classes);
                    return (string.IsNullOrEmpty(ns) ? "" : (ns + (string.IsNullOrEmpty(classes) ? "" : "::"))) + classes;
                }

                /// <summary>
                /// Get the fully qualified name
                /// </summary>

                public string GetQualifiedType(NamedDecl namedDecl)
                {
                    var classes = GetClasses();
                    return (string.IsNullOrEmpty(classes) ? "" : classes + "::") + namedDecl.Name;
                }

                /// <summary>
                /// Set the name of the class
                /// </summary>
                public void AddClass(string cls)
                {
                    m_classes.Add(cls);
                }

                /// <summary>
                ///  Remove the last class indirection
                /// </summary>
                public void RemoveClass()
                {
                    m_classes.RemoveAt(m_classes.Count - 1);
                }

                /// <summary>
                /// Add a new namespace indirection
                /// </summary>
                public void AddNamespace(string ns)
                {
                    m_namespaces.Add(ns);
                }

                /// <summary>
                /// Remove the last namespace indirection
                /// </summary>
                public void RemoveNamespace()
                {
                    m_namespaces.RemoveAt(m_namespaces.Count - 1);
                }

                private readonly List<string> m_namespaces = new List<string>();

                private readonly List<string> m_classes = new List<string>();
            }

            /// <summary>
            /// Extracted description of the hook
            /// </summary>
            public class HookDesc
            {
                /// <summary>
                /// Check if the 
                /// </summary>
                public StringMatcher Matcher;

                /// <summary>
                /// How many times was this description matched?
                /// </summary>
                public int NumMatches = 0;

                /// <summary>
                /// Description as provided by the user
                /// </summary>
                public readonly Configuration.HookT.DescriptionT Desc;

                /// <summary>
                /// Internal key into the descriptions map
                /// </summary>
                public readonly string Key;

                public HookDesc(KeyValuePair<string, Configuration.HookT.DescriptionT> kvp)
                {
                    Key = kvp.Key;
                    Desc = kvp.Value;
                }

                /// <summary>
                /// Does the provided name match the requested C function or class method?
                /// </summary>
                public bool Matches(string name)
                {
                    bool match = Matcher.IsMatch(name);
                    NumMatches += match ? 1 : 0;
                    return match;
                }
            }

            /// <summary>
            /// Functions we need to hook
            /// </summary>
            private readonly List<HookDesc> m_hookDescs = new List<HookDesc>();

            public ClangASTVisitor(CompilerContext ctx, Configuration config) : base(ctx)
            {
                m_config = config;
                foreach (var kvp in m_config.Hook.Descriptions)
                {
                    m_hookDescs.Add(new HookDesc(kvp)
                    {
                        Matcher = new StringMatcher(kvp.Value.Name),
                    });
                    kvp.Value.Input.ForEach(input => m_relevantInputFilenames.Add(input));
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
                    // Get the filename of the declaration
                    decl.Location.GetSpellingLocation(out CXFile cxfile, out uint line, out uint column, out uint offset);
                    var file = cxfile.Name.ToString();
                    m_inputFiles.Add(file);

                    // Only consider declarations form one of the specified input files
                    if (m_relevantInputFilenames.Contains(Path.GetFileName(file)))
                    {
                        Visit(decl, new TraversalData());
                    }
                }

                // Check we hooked all functions/classes
                foreach (var desc in m_hookDescs)
                {
                    if (desc.NumMatches == 0)
                    {
                        Error($"hook description not found '{desc.Desc.Name}'");
                    }
                }
                return m_hasError ? null : m_bir;
            }

            /// <summary>
            /// Main dispatch method
            /// </summary>
            private void Visit(Cursor cursor, TraversalData traversalData)
            {
                if (m_visitedCursors.Contains(cursor))
                {
                    // The AST can by cyclic
                    return;
                }
                m_visitedCursors.Add(cursor);

                if (cursor is Decl decl)
                {
                    VisitDecl(decl, traversalData);
                }
            }

            private void Visit(IReadOnlyList<Decl> decls, TraversalData traversalData)
            {
                foreach (var decl in decls)
                {
                    Visit(decl, traversalData);
                }
            }
            private void Visit(IReadOnlyList<Cursor> decls, TraversalData traversalData)
            {
                foreach (var decl in decls)
                {
                    Visit(decl, traversalData);
                }
            }

            /// <summary>
            /// Declaration dispatch
            /// </summary>
            private void VisitDecl(Decl decl, TraversalData traversalData)
            {
                if (decl.Kind == CXCursorKind.CXCursor_FirstDecl || decl.Kind == CXCursorKind.CXCursor_UnexposedDecl)
                {
                    Visit(decl.CursorChildren, traversalData);
                }
                else if (decl is NamespaceDecl namespaceDecl)
                {
                    VisitNamespaceDecl(namespaceDecl, traversalData);
                }
                else if (decl is FunctionDecl funDecl)
                {
                    VisitFunctionDecl(funDecl, traversalData);
                }
                else if (decl is TypeDecl typeDecl)
                {
                    VisitTypeDecl(typeDecl, traversalData);
                }
            }

            /// <summary>
            /// Visit a namespace declaration
            /// </summary>
            private void VisitNamespaceDecl(NamespaceDecl namespaceDecl, TraversalData traversalData)
            {
                traversalData.AddNamespace(namespaceDecl.ToString());
                Visit(namespaceDecl.Decls, traversalData);
                traversalData.RemoveNamespace();
            }

            /// <summary>
            /// Visit a namespace declaration
            /// </summary>
            private void VisitTypeDecl(TypeDecl typeDecl, TraversalData traversalData)
            {
                if (typeDecl.Kind == CXCursorKind.CXCursor_ClassDecl || typeDecl.Kind == CXCursorKind.CXCursor_StructDecl)
                {
                    traversalData.AddClass(typeDecl.Name);
                }

                Visit(typeDecl.CursorChildren, traversalData);

                if (typeDecl.Kind == CXCursorKind.CXCursor_ClassDecl || typeDecl.Kind == CXCursorKind.CXCursor_StructDecl)
                {
                    traversalData.RemoveClass();
                }
            }

            /// <summary>
            /// Visit a function declaration
            /// </summary>
            private void VisitFunctionDecl(FunctionDecl funDecl, TraversalData traversalData)
            {
                var desc = MatchHook(traversalData.GetQualifiedType(funDecl));
                if (desc != null)
                {
                    m_bir.Hooks.Add(CreateHook(funDecl, desc, traversalData));
                }
            }

            /// <summary>
            /// Check if need to hook this function
            /// </summary>
            private HookDesc MatchHook(string name)
            {
                foreach (var desc in m_hookDescs)
                {
                    if (desc.Matches(name))
                    {
                        return desc;
                    }
                }
                return null;
            }

            /// <summary>
            /// Register a hook
            /// </summary>
            private BIR.BIR.Hook CreateHook(FunctionDecl decl, HookDesc hookDesc, TraversalData traversalData)
            {
                BIR.BIR.Hook hook = new BIR.BIR.Hook();

                var desc = hookDesc.Desc;

                hook.Identifier = IO.MakeValidIdentifier(string.IsNullOrEmpty(desc.Identifier) ? traversalData.GetQualifiedType(decl).Replace("::", "_") : desc.Identifier);
                hook.ReturnType = decl.ReturnType.ToString();
                hook.Module = desc.Module;
                hook.Inputs = desc.Input;

                if (decl.Handle.Kind == CXCursorKind.CXCursor_CXXMethod)
                {
                    var cxxMethodDecl = (CXXMethodDecl)decl;
                    hook.VTableThisType = traversalData.GetClasses();
                    hook.HookType = cxxMethodDecl.IsVirtual ? BIR.BIR.HookTypeEnum.VTable : BIR.BIR.HookTypeEnum.CFunction;
                }
                else
                {
                    hook.HookType = BIR.BIR.HookTypeEnum.CFunction;
                    hook.CFunctionName = traversalData.GetQualifiedType(decl);
                    if (string.IsNullOrEmpty(hook.Module))
                    {
                        throw new Exception($"'hook.descriptions.[{hookDesc.Key}].module' is required to load function \"{desc.Name}\"");
                    }
                }

                // Extract parameter
                foreach (var param in decl.Parameters)
                {
                    hook.Parameters.Add(new BIR.BIR.Hook.Parameter()
                    {
                        Name = param.Name,
                        Type = param.Type.ToString(),
                    });
                }

                return hook;
            }

            private void Error(Decl decl, string message)
            {
                Error(message, new SourceRange(ExtractLocation(decl.Location)));
            }

            private void Error(string message, SourceRange range = null)
            {
                m_hasError = true;
                Diagnostics.Error(message, range);
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


        private static string Dump(Cursor c)
        {
            var stringBuilder = new StringBuilder();
            DumpImpl(c, stringBuilder, 0, "");
            return stringBuilder.ToString();
        }

        private static void DumpToFile(Cursor c, string file = "ast.txt")
        {
            File.WriteAllText(file, Dump(c));
        }

        private static void DumpImpl(Cursor c, StringBuilder stringBuilder, int indent, string prefix)
        {
            stringBuilder.AppendLine(new string(' ', Math.Max(0, indent - 2)) + prefix + c.KindSpelling + ": " + c.ToString());
            var numChildren = c.CursorChildren.Count;
            for (int i = 0; i < numChildren; ++i)
            {
                DumpImpl(c.CursorChildren[i], stringBuilder, indent + 2, i == numChildren - 1 ? "`-" : "|-");
            }
        }
    }
}
