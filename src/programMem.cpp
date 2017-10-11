#include "programMem.h"

#include "utils.h"

using namespace CoreIR;

void addProgramMem(CoreIR::Context* c, CoreIR::Namespace* global) {
  Params programMemParams({{"pcWidth", c->Int()}});

  TypeGen* memParamsTypeGen =
    global->newTypeGen("programMemTypeGen",
		       programMemParams,
		       [](Context* c, Values args) {
			 //uint pcWidth = args.at("pcWidth")->get<int>();
			 return c->Record();
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

      // Connect stage counter where needed

      // Connect memory rdata to registers

      // Set register enables

      // Connect PC to increments
      def->connect("pc.out", "inc1.in");
      def->connect("inc1.out", "inc2.in");

      def->connect("pc.out", "memMux.in.0");
      def->connect("inc1.out", "memMux.in.1");
      def->connect("inc2.out", "memMux.in.2");
      

    });
}
