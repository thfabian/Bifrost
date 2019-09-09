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
using System.Linq;
using CommandLine;
using Bifrost.Compiler.Input;
using CommandLine.Text;

using System.Reflection;
using System.Reflection.Emit;

namespace Bifrost.Compiler.Driver
{
    static class ClassBuilder
    {
        public struct PropertyDesc
        {
            public string Name;

            public Type Type;

            public Attribute Attr;
        }

        public static object CreateClass(string name, params PropertyDesc[] properties)
        {
            TypeBuilder dynamicClass = CreateClass(name);
            AddConstructor(dynamicClass);
            foreach (var property in properties)
            {
                AddProperty(dynamicClass, property);
            }
            Type type = dynamicClass.CreateType();
            return Activator.CreateInstance(type);
        }

        private static TypeBuilder CreateClass(string name)
        {
            AssemblyName assemblyName = new AssemblyName(name);
            AssemblyBuilder assemblyBuilder = AppDomain.CurrentDomain.DefineDynamicAssembly(assemblyName, AssemblyBuilderAccess.Run);
            ModuleBuilder moduleBuilder = assemblyBuilder.DefineDynamicModule("MainModule");
            TypeBuilder typeBuilder = moduleBuilder.DefineType(assemblyName.FullName,
                TypeAttributes.Public |
                TypeAttributes.Class |
                TypeAttributes.AutoClass |
                TypeAttributes.AnsiClass |
                TypeAttributes.BeforeFieldInit |
                TypeAttributes.AutoLayout, null);
            return typeBuilder;
        }

        private static void AddConstructor(TypeBuilder typeBuilder)
        {
            typeBuilder.DefineDefaultConstructor(MethodAttributes.Public | MethodAttributes.SpecialName | MethodAttributes.RTSpecialName);
        }

        private static void AddProperty(TypeBuilder typeBuilder, PropertyDesc property)
        {
            FieldBuilder fieldBuilder = typeBuilder.DefineField("_" + property.Name, property.Type, FieldAttributes.Private);
            PropertyBuilder propertyBuilder = typeBuilder.DefineProperty(property.Name, PropertyAttributes.HasDefault, property.Type, null);

            // Add attribute
            Type[] ctorParams = new Type[] { typeof(char), typeof(string) };
            CustomAttributeBuilder customAttributeBuilder = new CustomAttributeBuilder(
                        typeof(OptionAttribute).GetConstructor(ctorParams),
                        new object[] { 'f', "bar" },
                        property.Attr.GetType().GetFields(),
                        property.Attr.GetType().GetFields().Select(f => f.GetValue(property.Attr)).ToArray()
                        );
            propertyBuilder.SetCustomAttribute(customAttributeBuilder);

            // Add getter
            MethodBuilder getPropMthdBldr = typeBuilder.DefineMethod("get_" + property.Name, 
                MethodAttributes.Public | MethodAttributes.SpecialName | MethodAttributes.HideBySig,
                property.Type, Type.EmptyTypes);
            ILGenerator getIl = getPropMthdBldr.GetILGenerator();

            getIl.Emit(OpCodes.Ldarg_0);
            getIl.Emit(OpCodes.Ldfld, fieldBuilder);
            getIl.Emit(OpCodes.Ret);
            propertyBuilder.SetGetMethod(getPropMthdBldr);

            // Add setter
            MethodBuilder setPropMthdBldr = typeBuilder.DefineMethod("set_" + property.Name,
                  MethodAttributes.Public | MethodAttributes.SpecialName | MethodAttributes.HideBySig,
                  null, new[] { property.Type });

            ILGenerator setIl = setPropMthdBldr.GetILGenerator();
            Label modifyProperty = setIl.DefineLabel();
            Label exitSet = setIl.DefineLabel();

            setIl.MarkLabel(modifyProperty);
            setIl.Emit(OpCodes.Ldarg_0);
            setIl.Emit(OpCodes.Ldarg_1);
            setIl.Emit(OpCodes.Stfld, fieldBuilder);

            setIl.Emit(OpCodes.Nop);
            setIl.MarkLabel(exitSet);
            setIl.Emit(OpCodes.Ret);
            propertyBuilder.SetSetMethod(setPropMthdBldr);
        }
    }


    /// <summary>
    /// Invokes the compiler
    /// </summary>
    public static class Driver
    {
        public static int Run(IEnumerable<string> args)
        {
            int exitCode = 0;

            // Parse command-line
            var parser = new Parser(s =>
            {
                s.CaseSensitive = Parser.Default.Settings.CaseSensitive;
                s.CaseInsensitiveEnumValues = Parser.Default.Settings.CaseInsensitiveEnumValues;
                s.ParsingCulture = Parser.Default.Settings.ParsingCulture;
                s.HelpWriter = null;
                s.IgnoreUnknownArguments = Parser.Default.Settings.IgnoreUnknownArguments;
                s.AutoHelp = true;
                s.AutoVersion = false;
                s.EnableDashDash = false;
                s.MaximumDisplayWidth = Parser.Default.Settings.MaximumDisplayWidth;
            });

            var result = parser.ParseArguments(() =>
            {
                var c = new Configuration();
                var klass = ClassBuilder.CreateClass("Student", new ClassBuilder.PropertyDesc[] {
                    new ClassBuilder.PropertyDesc()
                    {
                        Name = "ID",
                        Type = typeof(bool),
                        Attr = c.GetType().GetFields().First().GetCustomAttribute(typeof(OptionAttribute))
                    }
                });
                return klass;
            }, args);
            result.WithParsed(c =>
            {
                Console.WriteLine(c);
            });
            result.WithNotParsed(errors =>
            {
                foreach (var err in errors)
                {
                    if (err.GetType() == typeof(HelpRequestedError))
                    {
                        var helpTxt = new HelpText
                        {
                            AddDashesToOption = true,
                            AutoVersion = false,
                            MaximumDisplayWidth = Console.WindowWidth,
                        };
                        helpTxt.AddPreOptionsText("bfc - Bifrost Compiler (0.0.1)\n\nOptions:");
                        helpTxt.AddOptions(result);
                        helpTxt.AddPostOptionsText("Example:\n  bfc.exe --help");

                        Console.WriteLine(helpTxt.ToString());
                    }
                    else
                    {
                        Console.WriteLine(err);
                    }
                }
                exitCode = 1;
            });
            return exitCode;
        }

        private static int RunCompiler(Configuration c)
        {
            return 0;
        }
    }
}
