using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Text;
using Xunit.Sdk;

namespace Bifrost.Compiler.Test.Utils
{
    /// <summary>
    /// Load a file from the assembly
    /// 
    /// When adding a file make sure that:
    ///   1) The file is an "embedded resource" (Properties -> Build Action)
    ///   2) The file is in the same folder as the .cs test file
    /// </summary>
    public class FileAttribute : DataAttribute
    {
        private readonly string m_filename;

        /// <summary>
        /// Get the associated file of this test
        /// </summary>
        public FileAttribute(string filename)
        {
            m_filename = filename;
        }

        /// <inheritDoc />
        public override IEnumerable<object[]> GetData(MethodInfo testMethod)
        {
            if (testMethod == null)
            {
                throw new ArgumentNullException(nameof(testMethod));
            }

            var assembly = Assembly.GetExecutingAssembly();

            // Read the file from the assembly
            var declType = testMethod.DeclaringType.ToString();
            var location = $"{declType.Substring(0, declType.LastIndexOf("."))}.{m_filename}";

            string path = m_filename;
            using (Stream stream = assembly.GetManifestResourceStream(location))
            {
                using (StreamReader reader = new StreamReader(stream))
                {
                    var content = reader.ReadToEnd();

                    // Write the file to a temp directory
                    path = Path.Combine(Path.GetTempPath(), m_filename);
                    File.WriteAllText(path, content);
                }
            }

            if (!File.Exists(path))
            {
                throw new ArgumentException($"Could not find file: {path}");
            }
            return new List<object[]>() { new object[] { path } };
        }
    }
}
