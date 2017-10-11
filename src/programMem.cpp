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
      //programMem->setGeneratorDefFromFun([](ModuleDef* def, Context* c, Type* t, Args args) {
      //uint pcWidth = args.at("pcWidth")->get<int>();
      
    });
}
