#include "programMem.h"

#include "utils.h"

using namespace CoreIR;

void addProgramMem(CoreIR::Context* c, CoreIR::Namespace* global) {
  Params programMemParams({{"pcWidth", AINT}});

  TypeGen* memParamsTypeGen =
    global->newTypeGen("programMemTypeGen",
		       programMemParams,
		       [](Context* c, Args args) {
			 //uint pcWidth = args.at("pcWidth")->get<int>();
			 return c->Record();
		       }
		       );

  Generator* programMem =
    global->newGeneratorDecl("programMem",
			     memParamsTypeGen,
			     programMemParams);

  programMem->setGeneratorDefFromFun([](ModuleDef* def, Context* c, Type* t, Args args) {
      //uint pcWidth = args.at("pcWidth")->get<int>();
      
    });
}
