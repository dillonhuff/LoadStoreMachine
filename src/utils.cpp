#include "utils.h"

using namespace CoreIR;

void addIncReset(Context* c, Namespace* global) {
  Params incResetParams({{"width", AINT}});

  TypeGen* incResetTypeGen =
    global->newTypeGen("IncResetTypeGen",
		       incResetParams,
		       [](Context* c, Args args) {
			 uint width = args.at("width")->get<int>();

			 return c->Record({
			     {"selectBit", c->BitIn()},
			       {"in", c->Array(width, c->BitIn())},
				 {"out", c->Array(width, c->Bit())}
			   });
		       }
		       );

  Generator* inc = global->newGeneratorDecl("incReset",
					    incResetTypeGen,
					    incResetParams);

  inc->setGeneratorDefFromFun([](ModuleDef* def,Context* c, Type* t, Args args) {
    uint width = args.at("width")->get<int>();
      
    def->addInstance("pcMultiplexer", "coreir.mux", {{"width", Const(width)}});
    def->addInstance("incrementer", "global.inc", {{"width", Const(width)}});
    Args wArg({{"width", Const(width)}});
    def->addInstance("resetConstant", "coreir.const", wArg, {{"value", Const(0)}});

    // Connections
    def->connect("self.selectBit", "pcMultiplexer.sel");
    def->connect("self.in", "incrementer.in");

    def->connect("incrementer.out", "pcMultiplexer.in0");
    def->connect("resetConstant.out", "pcMultiplexer.in1");

    def->connect("pcMultiplexer.out", "self.out");
    
  });
  
}

void addIncrementer(Context* c, Namespace* global) {

  Params incParams({{"width", AINT}});
  //Other param types: ABOOL,ASTRING,ATYPE

  TypeGen* incTypeGen = global->newTypeGen(
    "IncTypeGen", //name of typegen
    incParams, //Params required for typegen
    [](Context* c, Args args) { //lambda for generating the type

      uint width = args.at("width")->get<int>();

      return c->Record({
	  {"in", c->Array(width, c->BitIn())},
	    {"out",c->Array(width,c->Bit())},
      });
    } //end lambda
  ); //end newTypeGen

  ASSERT(global->hasTypeGen("IncTypeGen"),"Can check for typegens in namespaces");


  //Now lets create a generator declaration for our inc
  Generator* inc = global->newGeneratorDecl("inc", incTypeGen, incParams);
  //The third argument is the inc parameters. This needs to be a superset of the parameters for the typegen.
  
  //Now lets define our generator function. I am going to use a lambda again, but you could pass in
  //  a normal function with the same type signature.
  inc->setGeneratorDefFromFun([](ModuleDef* def,Context* c, Type* t, Args args) {
    //Similar to the typegen, lets extract the width;
    uint width = args.at("width")->get<int>();
      
    //Now just define the inc with with all the '16's replaced by 'width'
    Args wArg({{"width", Const(width)}});
    def->addInstance("ai","coreir.add",wArg);
    def->addInstance("ci","coreir.const",wArg,{{"value", Const(1)}});
    
    //Connections
    def->connect("ci.out","ai.in0");
    def->connect("self.in","ai.in1");
    def->connect("ai.out","self.out");
  }); //end lambda, end function

}

void addCounter(Context* c, Namespace* global) {
  //Now lets make our counter as a generator.
  //We want our Generator to be able to take in the parameter width.
  //We need to specify that the width is of type "int"
  Params counterParams({{"width",AINT}}); //"Arg Int". I know, its bad
  //Other param types: ABOOL,ASTRING,ATYPE

  //Instead of defining a type, now we need to define a type Generator. This allows CoreIR to statically type check all connections.
  TypeGen* counterTypeGen = global->newTypeGen(
    "CounterTypeGen", //name of typegen
    counterParams, //Params required for typegen
    [](Context* c, Args args) { //lambda for generating the type
      //Arg* widthArg = args.at("width"); //Checking for valid args is already done for you
      uint width = args.at("width")->get<int>(); //widthArg->get<int>(); //get function to extract the arg value.
      return c->Record({
        {"en",c->BitIn()}, 
        {"out",c->Array(width,c->Bit())}, //Note: Array is parameterized by width now
        {"clk",c->Named("coreir.clkIn")},
      });
    } //end lambda
  ); //end newTypeGen
  ASSERT(global->hasTypeGen("CounterTypeGen"),"Can check for typegens in namespaces");


  //Now lets create a generator declaration for our counter
  Generator* counter = global->newGeneratorDecl("counter",counterTypeGen,counterParams);
  //The third argument is the counter parameters. This needs to be a superset of the parameters for the typegen.
  
  //Now lets define our generator function. I am going to use a lambda again, but you could pass in
  //  a normal function with the same type signature.
  counter->setGeneratorDefFromFun([](ModuleDef* def,Context* c, Type* t, Args args) {
    //ModuleDef* def : The circuit you are defining.
    //Type* t: The generated Type of the counter (Using your counterTypeGen function!)
    //Args args: The arguments supplied to the instance of the counter.
    
    //Similar to the typegen, lets extract the width;
    uint width = args.at("width")->get<int>();
      
    //Now just define the counter with with all the '16's replaced by 'width'
    Args wArg({{"width", Const(width)}});
    def->addInstance("ai","coreir.add",wArg);
    def->addInstance("ci","coreir.const",wArg,{{"value", Const(1)}});
    //Reg has default arguments. en/clr/rst are False by default. Init is also 0 by default
    def->addInstance("ri","coreir.reg",{{"width", Const(width)},{"en", Const(true)}});
    
    //Connections
    def->connect("self.clk","ri.clk");
    def->connect("self.en","ri.en");
    def->connect("ci.out","ai.in0");
    def->connect("ai.out","ri.in");
    def->connect("ri.out","ai.in1");
    def->connect("ri.out","self.out");
  }); //end lambda, end function
  
}


