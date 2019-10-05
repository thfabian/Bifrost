using System;
using System.Collections.Generic;
using System.Text;

namespace Bifrost.Compiler.Core
{
    public static class DictionaryExtensions
    {
        public static void Update<TKey, TValue>(this Dictionary<TKey, TValue> dictionary, TKey key, TValue value)
        {
            IsNull(dictionary);
            if (dictionary.ContainsKey(key))
            {
                dictionary[key] = value;
            }
            else
            {
                dictionary.Add(key, value);
            }
        }

        private static void IsNull<TKey, TValue>(this Dictionary<TKey, TValue> dictionary)
        {
            if (dictionary == null)
            {
                throw new ArgumentNullException(nameof(dictionary));
            }
        }
    }
}
