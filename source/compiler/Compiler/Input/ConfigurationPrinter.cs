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
using System.Collections;
using System.Collections.Generic;
using System.Reflection;
using System.Text;
using System.Linq;

namespace Bifrost.Compiler.Input
{
    /// <summary>
    /// Print the configuration with default values and associated type
    /// </summary>
    public static class ConfigurationPrinter
    {
        /// <summary>
        /// Pretty print the configuration
        /// </summary>
        public static string Print(Configuration config)
        {
            var builder = new StringBuilder();
            PrintImpl(builder, config, 0);
            return builder.ToString();
        }

        private static void PrintImpl(StringBuilder builder, object obj, int indent, string prefix = null)
        {
            if (obj == null)
            {
                return;
            }

            int indentPerLevel = 2;

            prefix = prefix ?? "";
            var newIndent = indent + prefix.Length;

            var type = obj.GetType();
            bool firstProp = true;
            foreach (var prop in type.GetProperties())
            {
                string indentStr = new string(' ', newIndent);
                string indentStrWithPrefix = new string(' ', indent) + (firstProp ? prefix : new string(' ', prefix.Length));
                firstProp = false;

                var propValue = prop.GetValue(obj, null);
                var propName = char.ToLower(prop.Name[0]) + prop.Name.Substring(1);

                void printCollection(string typeName, IEnumerable collection, bool empty, string emptyCollection, Func<object, bool> handleItem, Func<object, string> printItem)
                {
                    if (empty)
                    {
                        builder.Append($"{indentStr}# <{typeName}>: {GetComment(prop, indentStr)}\n");
                        builder.Append($"{indentStrWithPrefix}{propName}: {emptyCollection}\n\n");
                    }
                    else
                    {
                        // Print the collection if it's a collection of fundamental types
                        bool collectionPrinted = false;

                        foreach (var item in collection)
                        {
                            if (!collectionPrinted)
                            {
                                builder.Append($"{indentStr}# <{typeName}>: {GetComment(prop, indentStr)}\n");
                                builder.Append($"{indentStrWithPrefix}{propName}:\n");
                                collectionPrinted = true;
                            }

                            if (handleItem(item))
                            {
                                builder.Append($"{indentStr}{printItem(item)}\n");
                            }
                            else
                            {
                                PrintImpl(builder, item, newIndent, collectionPrinted ? " - " : "  ");
                            }
                        }

                        if (collectionPrinted)
                        {
                            builder.Append("\n");
                        }
                    }
                }

                if (propValue is IList list)
                {
                    printCollection("list", list, list.Count == 0, "[]", (item) => item.GetType() == typeof(string), (item) =>
                    {
                        return $" - {GetValue(item)}";
                    });
                }
                else if (propValue is IDictionary dict)
                {
                    printCollection("dict", dict, dict.Count == 0, "{}", (item) => item.GetType() == typeof(KeyValuePair<string, string>), (item) =>
                    {
                        var kvp = (KeyValuePair<string, string>)item;
                        return $"{new string(' ', indentPerLevel)}{kvp.Key}: {GetValue(kvp.Value)}";
                    });
                }
                else
                {
                    // This will not cut-off System.Collections because of the first check
                    if (prop.PropertyType.Assembly == type.Assembly && !prop.PropertyType.IsEnum)
                    {
                        builder.Append($"{indentStr}#\n{indentStr}# {GetComment(prop, indentStr)}\n{indentStr}#\n");
                        builder.Append($"{indentStrWithPrefix}{propName}:\n");
                        PrintImpl(builder, prop.GetValue(obj, null), newIndent + indentPerLevel);
                    }
                    else
                    {
                        builder.Append($"{indentStr}# <{GetType(propValue.GetType())}>: {GetComment(prop, indentStr)}\n");
                        builder.Append($"{indentStrWithPrefix}{propName}: {GetValue(propValue)}\n\n");
                    }
                }
            }
        }

        private static string GetType(Type type)
        {
            if (type.IsEnum)
            {
                var enumValues = new List<string>();
                foreach (var enumValue in Enum.GetValues(type))
                {
                    enumValues.Add(enumValue.ToString());
                }
                return $"enum{{{string.Join(",", enumValues)}}}";
            }
            if (type == typeof(string))
            {
                return "string";
            }
            if (type == typeof(System.Version))
            {
                return "string";
            }
            return type.ToString();
        }

        private static string GetValue(object propValue)
        {
            if (propValue is string str)
            {
                return str.Count() > 0 ? str : "\"\"";
            }
            return propValue.ToString();
        }

        private static string GetComment(PropertyInfo prop, string indentStr)
        {
            var commentAttrt = prop.GetCustomAttributes(typeof(CommentAttribute), false).FirstOrDefault();
            if (commentAttrt != null)
            {
                var attr = (CommentAttribute)commentAttrt;
                var comment = attr.Comment.Replace("\n", "\n" + indentStr + "# ");
                if (attr.Importance == ImportanceEnum.Required)
                {
                    comment = $"[{attr.Importance}] " + comment;
                }
                return comment;
            }
            return "";
        }
    }
}
