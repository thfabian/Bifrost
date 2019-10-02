using System;
using System.Collections.Generic;
using System.Text;
using Bifrost.Compiler.BIRProducer;
using Bifrost.Compiler.BIRConsumer;
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
            bool success = true;
            using (var section = CreateSection("Executing configuration"))
            {
                // Produce the BIR
                var bir = ProduceBIR(config);
                if (bir == null)
                {
                    throw new Exception("no valid Bifrost Intermediate Representation (BIR) was produced");
                }

                // Consume the BIR
                success = ConsumeBIR(config, bir);

                section.Done();
            }
            return success;
        }

        private BIR.BIR ProduceBIR(Configuration config)
        {
            BIR.BIR bir = null;
            using (var section = CreateSection("Producing BIR"))
            {
                // Assemble the producers
                var producers = new List<IBIRProducer>();
                producers.Add(new Clang(Context));

                // Run the producers
                foreach (var producer in producers)
                {
                    if ((bir = producer.Produce(config)) != null)
                    {
                        break;
                    }
                }
                if (bir == null)
                {
                    return null;
                }

                // Let the producers visit the final BIR 
                foreach (var producer in producers)
                {
                    producer.Visit(config, bir);
                }

                section.Done();
            }
            return bir;
        }

        private bool ConsumeBIR(Configuration config, BIR.BIR bir)
        {
            bool success = true;
            using (var section = CreateSection("Consuming BIR"))
            {
                // Assemble the consumers
                var consumers = new List<IBIRConsumer>();
                consumers.Add(new BifrostPlugin(Context));

                // Let the producers visit the final BIR 
                foreach (var consumer in consumers)
                {
                    success &= consumer.Consume(config, bir);
                }

                section.Done();
            }
            return success;
        }
    }
}
