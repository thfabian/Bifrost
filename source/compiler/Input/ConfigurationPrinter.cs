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

        private static void PrintImpl(StringBuilder builder, object obj, int indent)
        {
            if (obj == null)
            {
                return;
            }

            int indentPerLevel = 2;

            string indentStr = new string(' ', indent);
            var type = obj.GetType();

            foreach (var prop in type.GetProperties())
            {
                var propValue = prop.GetValue(obj, null);
                var propName = char.ToLower(prop.Name[0]) + prop.Name.Substring(1);

                void printCollection(string typeName, IEnumerable collection, bool empty, string emptyCollection, Func<object, bool> handleItem, Func<object, string> printItem)
                {
                    if (empty)
                    {
                        builder.Append($"{indentStr}# <{typeName}>: {GetComment(prop)}\n");
                        builder.Append($"{indentStr}{propName}: {emptyCollection}\n\n");
                    }
                    else
                    {
                        bool collectionPrinted = false;
                        foreach (var item in collection)
                        {
                            if (handleItem(item))
                            {
                                if (!collectionPrinted)
                                {
                                    builder.Append($"{indentStr}# <{typeName}>: {GetComment(prop)}\n");
                                    builder.Append($"{indentStr}{propName}:\n");
                                    collectionPrinted = true;
                                }
                                builder.Append($"{indentStr}{printItem(item)}\n");
                            }
                            else
                            {
                                PrintImpl(builder, item, indent + 3);
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
                    printCollection("list", list, list.Count == 0, "[]", (item) => item.GetType() == typeof(string), (item) => $" - {GetValue(item)}");
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
                    if (prop.PropertyType.Assembly == type.Assembly)
                    {
                        builder.Append($"{indentStr}#\n{indentStr}# {GetComment(prop)}\n{indentStr}#\n");
                        builder.Append($"{indentStr}{propName}:\n");
                        PrintImpl(builder, prop.GetValue(obj, null), indent + indentPerLevel);
                    }
                    else
                    {
                        builder.Append($"{indentStr}# <{GetType(propValue.GetType())}>: {GetComment(prop)}\n");
                        builder.Append($"{indentStr}{propName}: {GetValue(propValue)}\n\n");
                    }
                }
            }
        }

        private static string GetType(Type type)
        {
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

        private static string GetComment(PropertyInfo prop)
        {
            var comment = prop.GetCustomAttributes(typeof(CommentAttribute), false).FirstOrDefault();
            if (comment != null)
            {
                return ((CommentAttribute)comment).Comment;
            }
            return "";
        }
    }
}
