using System;
using System.Collections.Generic;
using System.Text;
using Bifrost.Compiler.Frontend;
using Bifrost.Compiler.Input;

namespace Bifrost.Compiler.Core
{
    /// <summary>
    /// Compile a given configuration
    /// </summary>
    public class Compiler : CompilerObject
    {
        public Compiler(CompilerContext ctx) : base(ctx) { }

        /// <summary>
        /// Run the given configuration
        /// </summary>
        public bool Run(Configuration config)
        {
            using (var section = CreateSection("Executing configuration"))
            {
                // Run the frontend actions
                var bir = ProduceBIR(config);
                if (bir == null)
                {
                    throw new Exception("no valid Bifrost Intermediate Representation (BIR) was produced");
                }
                section.Done();
            }
            return true;
        }

        private BIR.BIR ProduceBIR(Configuration config)
        {
            using (var section = CreateSection("Producing BIR"))
            {
                // Assemble the frontend actions
                var frontendActions = new List<IFrontendAction>();
                frontendActions.Add(new Clang(Context));

                // Run the frontend actions
                BIR.BIR bir = null;
                foreach (var action in frontendActions)
                {
                    if ((bir = action.Produce(config)) != null)
                    {
                        break;
                    }
                }
                if (bir == null)
                {
                    return null;
                }

                // Let the frontend actions visit the BIR 
                foreach (var action in frontendActions)
                {
                    action.Visit(config, bir);
                }

                section.Done();
            }
            return null;
        }
    }
}
