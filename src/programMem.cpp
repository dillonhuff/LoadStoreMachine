#include "programMem.h"

#include "utils.h"

using namespace CoreIR;

void addProgramMem(CoreIR::Context* c, CoreIR::Namespace* global) {
  Params programMemParams({{"pcWidth", c->Int()}});

  TypeGen* memParamsTypeGen =
    global->newTypeGen("programMemTypeGen",
		       programMemParams,
		       [](Context* c, Values args) {
			 uint pcWidth = args.at("pcWidth")->get<int>();
			 uint numStages = args.at("numStages")->get<int>();
			 uint stageBits = bitsNeededToStore(numStages);
			 return c->Record({
			     {"clk", c->Named("coreir.clkIn")},
			       {"stageNumber", c->Array(stageBits, c->BitIn())},

			       // Outputs
				 {"opName", c->Array(pcWidth, c->Bit())},
				   {"arg0", c->Array(pcWidth, c->Bit())},
				     {"arg1", c->Array(pcWidth, c->Bit())}
			   });
		       }
		       );

  Generator* programMem =
    global->newGeneratorDecl("programMem",
			     memParamsTypeGen,
			     programMemParams);

  programMem->setGeneratorDefFromFun([](Context* c, Values args, ModuleDef* def) {
      uint pcWidth = args.at("pcWidth")->get<int>();

      def->addInstance("PC",
		       "coreir.reg",
		       {{"width", Const::make(c, pcWidth)},
			   {"en", Const::make(c, true)}});

      def->addInstance("inc1", "global.inc", {{"width", Const::make(c, pcWidth)}});
      def->addInstance("inc2", "global.inc", {{"width", Const::make(c, pcWidth)}});

      def->addInstance("memMux",
		       "commonlib.muxN",
		       {{"N", Const::make(c, 3)},
			   {"width", Const::make(c, pcWidth)}}
		       );
      
      def->addInstance("opCode",
		       "coreir.reg",
		       {{"width", Const::make(c, pcWidth)},
			   {"en", Const::make(c, true)}});

      def->addInstance("opArg0",
		       "coreir.reg",
		       {{"width", Const::make(c, pcWidth)},
			   {"en", Const::make(c, true)}});

      def->addInstance("opArg1",
		       "coreir.reg",
		       {{"width", Const::make(c, pcWidth)},
			   {"en", Const::make(c, true)}});

      def->addInstance("progMem",
		       "coreir.mem",
		       {{"width", Const::make(c, pcWidth)},
			   {"depth", Const::make(c, pow(2, pcWidth))}});

      // Connect clock to sequential elements
      def->connect("self.clk", "progMem.clk");
      def->connect("self.clk", "PC.clk");
      def->connect("self.clk", "opName.clk");
      def->connect("self.clk", "arg0.clk");
      def->connect("self.clk", "arg1.clk");

      // Connect stage counter to enables where needed
      // Set register enables, this is the same as stage connection?

      // Connect memory rdata to registers
      def->connect("progMem.rdata", "opCode.in");
      def->connect("progMem.rdata", "arg0.in");
      def->connect("progMem.rdata", "arg1.in");

      // Connect PC to increments
      def->connect("pc.out", "inc1.in");
      def->connect("inc1.out", "inc2.in");

      def->connect("pc.out", "memMux.in.0");
      def->connect("inc1.out", "memMux.in.1");
      def->connect("inc2.out", "memMux.in.2");

      // Connect results to outputs
      def->connect("opName.out", "self.opName");
      def->connect("arg0.out", "self.arg0");
      def->connect("arg1.out", "self.arg1");
      
    });

  // Q: Can you pipeline a design just by merging some stage numbers in
  // modules that have stage number ports?
}
