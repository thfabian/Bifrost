using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Diagnostics;
using System.Text.RegularExpressions;
using System.Linq;

namespace Bifrost.Compiler.Core
{
    /// <summary>
    /// I/O related functions
    /// </summary>
    class IO : CompilerObject
    {
        public IO(CompilerContext context) : base(context)
        {
        }

        /// <summary>
        /// Create a temporary file <paramref name="filename"/>
        /// </summary>
        /// <param name="filename">Name of the file</param>
        /// <returns>Full path to the file</returns>
        public string CreateTempFile(string filename)
        {
            var path = Path.Combine(CreateTempDirectory(), filename);
            Logger.DebugFormat("Creating temporary file \"{0}\"", path);
            System.IO.File.Create(path).Dispose();
            return path;
        }

        /// <summary>
        /// Create a temporary directory
        /// </summary>
        /// <returns>Full path to the directory</returns>
        public string CreateTempDirectory()
        {
            return CreateDirectory(Path.GetTempPath(), Utils.UUID.Generate());
        }

        /// <summary>
        /// Copy file <paramref name="sourceFile"/> to <paramref name="destFile"/>
        /// </summary>
        public void CopyFile(string sourceFile, string destFile, bool overwrite = true)
        {
            TransferFileImpl(TransferMode.Copy, sourceFile, destFile, overwrite, true);
        }

        /// <summary>
        /// Copy file <paramref name="sourceFile"/> to <paramref name="destFile"/> if it exists
        /// </summary>
        public void CopyFileIfExists(string sourceFile, string destFile, bool overwrite = true)
        {
            if (System.IO.File.Exists(sourceFile))
            {
                TransferFileImpl(TransferMode.Copy, sourceFile, destFile, overwrite, true);
            }
        }

        /// <summary>
        /// Copy each file from it's source (given by the key) to it's destination (given by the value)
        /// </summary>
        public void CopyFiles(IDictionary<string, string> files)
        {
            TransferFilesImpl(TransferMode.Copy, files, true, true);
        }

        /// <summary>
        /// Copy all <paramref name="files"/> to <paramref name="destDirName"/>
        /// </summary>
        public void CopyFiles(IEnumerable<string> files, string destDirName, bool overwrite = true)
        {
            TransferFilesImpl(TransferMode.Copy, files.ToDictionary(x => x, x => Path.Combine(destDirName, Path.GetFileName(x))), overwrite, true);
        }

        /// <summary>
        /// Move file <paramref name="sourceFile"/> to <paramref name="destFile"/>
        /// </summary>
        public void MoveFile(string sourceFile, string destFile)
        {
            TransferFileImpl(TransferMode.Move, sourceFile, destFile, true, true);
        }

        /// <summary>
        /// Move all <paramref name="files"/> to <paramref name="destDirName"/>
        /// </summary>
        public void MoveFiles(IEnumerable<string> files, string destDirName)
        {
            TransferFilesImpl(TransferMode.Move, files.ToDictionary(x => x, x => Path.Combine(destDirName, Path.GetFileName(x))), true, true);
        }

        /// <summary>
        /// Move each file from it's source (given by the key) to it's destination (given by the value)
        /// </summary>
        public void MoveFiles(IDictionary<string, string> files)
        {
            TransferFilesImpl(TransferMode.Move, files, true, true);
        }

        /// <summary>
        /// Move all files matching any of <paramref name="searchPatterns"/> from <paramref name="sourceDirName"/> to <paramref name="destDirName"/>
        /// </summary>
        /// <param name="searchPatterns">Search strings to match against the names of files in <paramref name="sourceDirName"/>. This parameter can contain a combination 
        /// of valid literal path and wildcard (* and ?) characters, but doesn't support regular expressions</param>
        public void MoveFilesIf(string sourceDirName, string destDirName, params string[] searchPatterns)
        {
            var timer = Stopwatch.StartNew();
            var numFiles = 0;

            Logger.DebugFormat("Moving files matching \"{0}\" from \"{1}\" to \"{2}\" ...", System.String.Join("\", \"", searchPatterns), sourceDirName, destDirName);
            foreach (var pattern in searchPatterns)
            {
                foreach (FileInfo file in new DirectoryInfo(sourceDirName).GetFiles(pattern))
                {
                    numFiles += 1;
                    var destFile = Path.Combine(destDirName, file.Name);
                    if (System.IO.File.Exists(destFile))
                    {
                        System.IO.File.Delete(destFile);
                    }
                    MoveFile(file.FullName, Path.Combine(destDirName, file.Name));
                }
            }
            Logger.DebugFormat("Moved {0} files from \"{1}\" to \"{2}\" (took {3,5:F2}s)", numFiles, sourceDirName, destDirName, timer.ElapsedMilliseconds / 1000.0);
        }

        /// <summary>
        /// Get the full path to all files matching <paramref name="searchPattern"/> in <paramref name="sourceDirName"/>
        /// </summary>
        /// <param name="searchPattern">Search string to match against the names of files in <paramref name="sourceDirName"/>. This parameter can contain a combination 
        /// of valid literal path and wildcard (* and ?) characters, but doesn't support regular expressions</param>
        public List<string> GetFilesRecursive(string sourceDirName, string searchPattern)
        {
            var files = new List<string>();
            files.AddRange(GetFiles(sourceDirName, searchPattern));
            foreach (var directory in new DirectoryInfo(sourceDirName).GetDirectories())
            {
                files.AddRange(GetFilesRecursive(directory.FullName, searchPattern));
            }

            return files;
        }

        /// <summary>
        /// Check if directory <paramref name="sourceDirName"/> is empty
        /// </summary>
        public bool IsDirectoryEmpty(string sourceDirName)
        {
            return GetFiles(sourceDirName, "*").Count == 0;
        }


        /// <summary>
        /// Get the full path to all files matching <paramref name="searchPattern"/> in <paramref name="sourceDirName"/>
        /// </summary>
        /// <param name="searchPattern">Search string to match against the names of files in <paramref name="sourceDirName"/>. This parameter can contain a combination 
        /// of valid literal path and wildcard (* and ?) characters, but doesn't support regular expressions</param>
        public List<string> GetFiles(string sourceDirName, params string[] searchPattern)
        {
            return GetFilesImpl(sourceDirName, new List<string>(searchPattern), false);
        }

        /// <summary>
        /// Get the full path to all directories matching <paramref name="searchPattern"/> in <paramref name="sourceDirName"/>
        /// </summary>
        /// <param name="searchPattern">Search string to match against the names of directories in <paramref name="sourceDirName"/>. This parameter can contain a combination 
        /// of valid literal path and wildcard (* and ?) characters, but doesn't support regular expressions</param>
        public List<string> GetDirectories(string sourceDirName, string searchPattern, bool throwOnError = false)
        {
            var dirs = new List<string>();
            if (Directory.Exists(Path.Combine(sourceDirName, searchPattern)))
            {
                dirs.Add(Path.Combine(sourceDirName, searchPattern));
            }
            else
            {
                try
                {
                    foreach (DirectoryInfo dir in new DirectoryInfo(sourceDirName).GetDirectories(searchPattern))
                    {
                        dirs.Add(dir.FullName);
                    }
                }
                catch (Exception ex)
                {
                    if (throwOnError)
                    {
                        throw ex;
                    }
                    else
                    {
                        Logger.Warn("Unexpected exception", ex);
                        return dirs;
                    }
                }
            }
            return dirs;
        }


        /// <summary>
        /// Get the full path to all directories matching <paramref name="searchPattern"/> in <paramref name="sourceDirName"/>
        /// </summary>
        /// <param name="searchPattern">Search string to match against the names of directories in <paramref name="sourceDirName"/>. This parameter can contain a combination 
        /// of valid literal path and wildcard (* and ?) characters, but doesn't support regular expressions</param>
        public List<string> GetDirectoriesRecursive(string sourceDirName, string searchPattern, bool throwOnError = false)
        {
            var dirs = new List<string>();
            dirs.AddRange(GetDirectories(sourceDirName, searchPattern));
            foreach (var directory in new DirectoryInfo(sourceDirName).GetDirectories())
            {
                dirs.AddRange(GetDirectoriesRecursive(directory.FullName, searchPattern));
            }
            return dirs;
        }

        /// <summary>
        /// Get a list of all files in <paramref name="path"/>
        /// </summary>
        public List<string> GetFilesInDirectory(string path)
        {
            return GetFiles(path, "*");
        }

        // <summary>
        /// Make <paramref name="path"/> relative to <paramref name="baseFolder"/>
        /// </summary>
        public string MakeRelative(string baseFolder, string path)
        {
            // Folders must end in a slash
            if (!baseFolder.EndsWith(Path.DirectorySeparatorChar.ToString()))
            {
                baseFolder += Path.DirectorySeparatorChar;
            }
            return Uri.UnescapeDataString(new Uri(baseFolder).MakeRelativeUri(new Uri(path)).ToString().Replace('/', Path.DirectorySeparatorChar));
        }

        /// <summary>
        /// Replace all illegal characters (as well as " ") in <paramref name="filename"/> with <paramref name="replaceChar"/>
        /// </summary>
        public string MakeValidFilename(string filename, string replaceChar = "_")
        {
            var f = new string(filename);
            foreach (var c in (" " + "/" + new string(Path.GetInvalidFileNameChars()) + new string(Path.GetInvalidPathChars())))
            {
                f = f.Replace(c.ToString(), replaceChar);
            }
            return f;
        }

        /// <summary>
        /// Check if the regular expression <paramref name="pattern"/> can be found in the content of <paramref name="files"/>
        /// </summary>
        /// <param name="pattern">Regex to apply to the content spanning lines [0, <paramref name="numLinesFromStart"/>)</param>
        /// <param name="files">Files to analyze</param>
        /// <param name="numLinesFromStart">Specifies the number of lines from the beginning of a file or other item. The default is -1 (all lines)</param>
        /// <returns>True if the <paramref name="pattern"/> has been found at least once, false otherwise</returns>
        public bool FindString(string pattern, IEnumerable<string> files, int numLinesFromStart = -1)
        {
            return FindNumStrings(pattern, files, numLinesFromStart, 1);
        }
        public bool FindString(string pattern, string file, int numLinesFromStart = -1)
        {
            return FindString(pattern, new string[] { file }, numLinesFromStart);
        }

        /// <summary>
        /// Like FindString but returns the path to the file (or null if no match was found)
        /// </summary>
        public string FindStringAndGetMatch(string pattern, IEnumerable<string> files, int numLinesFromStart = -1)
        {
            var matches = FindNumStringsAndGetMatches(pattern, files, numLinesFromStart, 1);
            return matches != null && matches.Count() == 1 ? matches.First() : null;
        }

        /// <summary>
        /// Check if the regular expression <paramref name="pattern"/> can be found at elast <paramref name="num"/> times in the content of <paramref name="files"/>
        /// </summary>
        /// <param name="pattern">Regex to apply to the content of each file</param>
        /// <param name="files">Files to analyze</param>
        /// <param name="numLinesFromStart">Specifies the number of lines from the beginning of a file or other item. The default is -1 (all lines)</param>
        /// <returns>True if the <paramref name="pattern"/> has been found at least once, false otherwise</returns>
        public bool FindNumStrings(string pattern, IEnumerable<string> files, int numLinesFromStart, int num, bool allowMultiplePerFile = false)
        {
            return FindNumStringsImpl(pattern, files, numLinesFromStart, num, allowMultiplePerFile).Item1;
        }

        /// <summary>
        /// Like FindString but returns the path to the file (or null if less than <paramref name="num"/> were found)
        /// </summary>
        public IEnumerable<string> FindNumStringsAndGetMatches(string pattern, IEnumerable<string> files, int numLinesFromStart, int num, bool allowMultiplePerFile = false)
        {
            var matches = FindNumStringsImpl(pattern, files, numLinesFromStart, num, allowMultiplePerFile);
            if (!matches.Item1) return null;
            return matches.Item2;
        }


        /// <summary>
        /// Create all directory and subdirectories of path given by combining <paramref name="dirParts"/>
        /// </summary>
        /// <param name="dirParts">Path parts to combine to form the the directory</param>
        /// <returns>Full path to the created directory</returns>
        public string CreateDirectory(params string[] dirParts)
        {
            return CreateDirectoryImpl(true, dirParts);
        }

        /// <summary>
        /// Clear the directory <paramref name="dir"/> by erasing all files and subdirectories
        /// </summary>
        /// <param name="dir">Directory to clear</param>
        public void ClearDirectory(string dir, bool includeRootFolder = false)
        {
            var timer = Stopwatch.StartNew();
            Logger.DebugFormat("Clearing directory \"{0}\" ...", dir);
            DirectoryInfo dirInfo = new DirectoryInfo(dir);
            foreach (var f in dirInfo.GetFiles())
            {
                f.Delete();
            }
            foreach (var d in dirInfo.GetDirectories())
            {
                d.Delete(true);
            }

            if (includeRootFolder)
            {
                Directory.Delete(dir);
            }

            Logger.DebugFormat("Done clearing directory \"{0}\" (took {1,5:F2}s)", dir, timer.ElapsedMilliseconds / 1000.0);
        }

        /// <summary>
        /// Normalize the path to only contain '/' (instead of '\')
        /// </summary>
        public string Normalize(string path)
        {
            return path.Replace("\\", "/");
        }

        /// <summary>
        /// Normalize the path to only contain '\' (instead of '/')
        /// </summary>
        public string NormalizeToWinPath(string path)
        {
            return path.Replace("/", "\\");
        }

        /// <summary>
        /// Try to clean the directory. Refer to ClearDirectory documentation.
        /// </summary>
        public bool TryClearDirectory(string dir, bool includeRootFolder = false)
        {
            try
            {
                ClearDirectory(dir, includeRootFolder);
            }
            catch (Exception)
            {
                return false;
            }
            return true;
        }

        /// <summary>
        /// Try to remove the file <paramref name="path"/>
        /// </summary>
        /// <returns>1 if the file was removed, 0 otherwise</returns>
        public int TryRemoveFile(string path)
        {
            return TryRemoveFileImpl(path);
        }

        /// <summary>
        /// Try to remove the files <paramref name="files"/>
        /// </summary>
        /// <returns>the number of removed files</returns>
        public int TryRemoveFiles(IEnumerable<string> files)
        {
            int numRemoved = 0;
            foreach (var file in files)
            {
                numRemoved += TryRemoveFileImpl(file);
            }
            return numRemoved;
        }


        /// <summary>
        /// Copy directory from <paramref name="sourceDirName"/> to <paramref name="destDirName"/>
        /// </summary>
        public void CopyDirectory(string sourceDirName, string destDirName, bool copySubDirs = true, bool overwrite = true, bool verbosity = false)
        {
            var timer = Stopwatch.StartNew();
            Logger.DebugFormat("Copying directory \"{0}\" to \"{1}\" ...", sourceDirName, destDirName);
            TransferDirectoryImpl(TransferMode.Copy, sourceDirName, destDirName, copySubDirs, overwrite, verbosity);
            Logger.DebugFormat("Done copying directory \"{0}\" to \"{1}\" (took {2,5:F2}s)", sourceDirName, destDirName, timer.ElapsedMilliseconds / 1000.0);
        }

        /// <summary>
        /// Move directory from <paramref name="sourceDirName"/> to <paramref name="destDirName"/>
        /// </summary>
        public void MoveDirectory(string sourceDirName, string destDirName, bool moveSubDirs = true, bool verbosity = false)
        {
            var timer = Stopwatch.StartNew();
            Logger.DebugFormat("Moving directory \"{0}\" to \"{1}\" ...", sourceDirName, destDirName);
            TransferDirectoryImpl(TransferMode.Move, sourceDirName, destDirName, moveSubDirs, true, verbosity);
            Logger.DebugFormat("Done moving directory \"{0}\" to \"{1}\" (took {2,5:F2}s)", sourceDirName, destDirName, timer.ElapsedMilliseconds / 1000.0);
        }

        /// <summary>
        /// Try to move directory from <paramref name="sourceDirName"/> to <paramref name="destDirName"/> or copy it
        /// </summary>
        public void TryMoveOrCopyDirectory(string sourceDirName, string destDirName, bool moveSubDirs = true, bool verbosity = false)
        {
            try
            {
                MoveDirectory(sourceDirName, destDirName, moveSubDirs, verbosity);
            }
            catch (IOException)
            {
                CopyDirectory(sourceDirName, destDirName, moveSubDirs, verbosity);
            }
        }

        #region private
        private enum TransferMode
        {
            Copy,
            Move,
        }

        private List<string> GetFilesImpl(string sourceDirName, List<string> searchPattern, bool throwOnError)
        {
            var files = new List<string>();

            if (searchPattern.Count() == 1 && System.IO.File.Exists(searchPattern[0]))
            {
                files.Add(searchPattern[0]);
            }
            else
            {
                try
                {
                    foreach (var pattern in searchPattern)
                    {
                        foreach (FileInfo file in new DirectoryInfo(sourceDirName).GetFiles(pattern))
                        {
                            files.Add(Path.Combine(file.DirectoryName, file.Name));
                        }
                    }
                }
                catch (Exception ex)
                {
                    if (throwOnError)
                    {
                        throw ex;
                    }
                    else
                    {
                        Logger.Warn("Unexpected exception", ex);
                        return files;
                    }
                }
            }
            return files;
        }

        private (bool, IEnumerable<string>) FindNumStringsImpl(string pattern, IEnumerable<string> files, int numLinesFromStart, int num, bool allowMultiplePerFile = false)
        {
            var regex = new Regex(pattern);
            int n = 0;
            HashSet<string> matchedFiles = new HashSet<string>();

            foreach (var file in files)
            {
                if (allowMultiplePerFile)
                {
                    foreach (var line in numLinesFromStart == -1 ? System.IO.File.ReadAllLines(file) : System.IO.File.ReadLines(file).Take(numLinesFromStart).ToArray())
                    {
                        if (regex.Match(line).Success)
                        {
                            matchedFiles.Add(file);
                            n += 1;
                        }
                        if (n == num)
                        {
                            return (true, matchedFiles);
                        }
                    }
                }
                else
                {
                    var content = numLinesFromStart == -1 ? System.IO.File.ReadAllText(file) : System.String.Join("\n", System.IO.File.ReadLines(file).Take(numLinesFromStart).ToList());
                    if (regex.Match(content).Success)
                    {
                        matchedFiles.Add(file);
                        n += 1;
                    }
                    if (n == num)
                    {
                        return (true, matchedFiles);
                    }
                }

            }
            return (false, matchedFiles);
        }

        private string CreateDirectoryImpl(bool verbosity, params string[] dirParts)
        {
            var dir = Path.Combine(dirParts);
            if (Directory.Exists(dir))
            {
                return dir;
            }

            if (verbosity)
            {
                Logger.DebugFormat("Creating directory \"{0}\"", dir);
            }
            Directory.CreateDirectory(dir);
            return dir;
        }

        private void TransferDirectoryImpl(TransferMode mode, string sourceDirName, string destDirName, bool transferSubDirs, bool overwrite, bool verbosity)
        {
            DirectoryInfo dir = new DirectoryInfo(sourceDirName);

            if (!dir.Exists)
            {
                throw new DirectoryNotFoundException($"Source directory does not exist or could not be found: \"{sourceDirName}\"");
            }

            if (!Directory.Exists(destDirName))
            {
                CreateDirectoryImpl(false, destDirName);
            }

            // Copy files
            TransferFilesImpl(mode, dir.GetFiles().ToDictionary(f => f.FullName, f => Path.Combine(destDirName, f.Name)), overwrite, verbosity);

            // Copy directories
            if (transferSubDirs)
            {
                foreach (DirectoryInfo subdir in dir.GetDirectories())
                {
                    string temppath = Path.Combine(destDirName, subdir.Name);
                    TransferDirectoryImpl(mode, subdir.FullName, temppath, transferSubDirs, overwrite, verbosity);
                }
            }
        }

        private int TryRemoveFileImpl(string path)
        {
            Logger.Debug($"Trying to remove file \"{path}\"");
            if (!System.IO.File.Exists(path))
            {
                return 0;
            }

            try
            {
                System.IO.File.Delete(path);
            }
            catch (Exception e)
            {
                Logger.Warn($"Failed to remove file \"{path}\"", e);
                return 0;
            }

            return 1;
        }

        private void TransferFileImpl(TransferMode mode, string sourceFile, string destFile, bool overwrite, bool verbosity)
        {
            if (verbosity)
            {
                Logger.DebugFormat("{0}ing file \"{1}\" to \"{2}\"", mode, sourceFile, destFile);
            }

            var exists = System.IO.File.Exists(destFile);
            if (!overwrite && exists)
            {
                return;
            }

            switch (mode)
            {
                case TransferMode.Copy:
                    System.IO.File.Copy(sourceFile, destFile, overwrite);
                    break;
                case TransferMode.Move:
                    System.IO.File.Move(sourceFile, destFile);
                    break;
                default:
                    break;
            }
        }

        private void TransferFilesImpl(TransferMode mode, IDictionary<string, string> files, bool overwrite, bool verbosity)
        {
            if (files.Count == 0)
            {
                return;
            }

            var timer = Stopwatch.StartNew();
            var filesCopied = 0;
            var filesCopiedSinceLastReset = 0;

            foreach (var sourceDestPair in files)
            {
                filesCopied += 1;
                filesCopiedSinceLastReset += 1;

                var src = sourceDestPair.Key;
                var srcDir = Path.GetDirectoryName(src);

                var dst = sourceDestPair.Value;
                var dstDir = Path.GetDirectoryName(dst);

                if (verbosity && timer.ElapsedMilliseconds > 2500)
                {
                    var filesPerSecond = filesCopiedSinceLastReset / timer.Elapsed.TotalSeconds;

                    Logger.DebugFormat("{0}ing files \"{1}\" to {2}: [{3,4}/{4} files] {5,3} % ({6} files/s)", mode, srcDir, dstDir, filesCopied, files.Count, (int)(((double)filesCopied / files.Count) * 100), (int)filesPerSecond);
                    timer.Restart();
                    filesCopiedSinceLastReset = 0;
                }

                CreateDirectoryImpl(verbosity, dstDir);
                TransferFileImpl(mode, src, dst, overwrite, false);
            }
        }
        #endregion
    }
}
