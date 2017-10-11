#include "utils.h"

using namespace CoreIR;
using namespace std;

int bitsNeededToStore(const int maxVal) {
  return floor(log2(maxVal)) + 1;  
}

void addIncReset(Context* c, Namespace* global) {
  Params incResetParams({{"width", c->Int()}});

  TypeGen* incResetTypeGen =
    global->newTypeGen("IncResetTypeGen",
		       incResetParams,
		       [](Context* c, Values args) {
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

  inc->setGeneratorDefFromFun([](Context* c, Values args, ModuleDef* def) {
    uint width = args.at("width")->get<int>();
      
    def->addInstance("pcMultiplexer", "coreir.mux", {{"width", Const::make(c, width)}});
    def->addInstance("incrementer", "global.inc", {{"width", Const::make(c, width)}});
    Values wArg({{"width", Const::make(c, width)}});
    def->addInstance("resetConstant", "coreir.const", wArg, {{"value", Const::make(c, BitVector(width, 0))}});

    // Connections
    def->connect("self.selectBit", "pcMultiplexer.sel");
    def->connect("self.in", "incrementer.in");

    def->connect("incrementer.out", "pcMultiplexer.in0");
    def->connect("resetConstant.out", "pcMultiplexer.in1");

    def->connect("pcMultiplexer.out", "self.out");
    
  });
  
}

void addIncrementer(Context* c, Namespace* global) {

  Params incParams({{"width", c->Int()}});
  //Other param types: ABOOL,ASTRING,ATYPE

  TypeGen* incTypeGen = global->newTypeGen(
    "IncTypeGen", //name of typegen
    incParams, //Params required for typegen
    [](Context* c, Values args) { //lambda for generating the type

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
  inc->setGeneratorDefFromFun([](Context* c, Values args, ModuleDef* def) {
      //inc->setGeneratorDefFromFun([](ModuleDef* def,Context* c, Type* t, Values args) {
    //Similar to the typegen, lets extract the width;
    uint width = args.at("width")->get<int>();
      
    //Now just define the inc with with all the '16's replaced by 'width'
    Values wArg({{"width", Const::make(c, width)}});
    def->addInstance("ai","coreir.add",wArg);
    def->addInstance("ci","coreir.const",wArg,{{"value", Const::make(c, BitVector(width, 1))}});
    
    //Connections
    def->connect("ci.out","ai.in0");
    def->connect("self.in","ai.in1");
    def->connect("ai.out","self.out");
  }); //end lambda, end function

}

void addCounter(Context* c, Namespace* global) {

  Params counterParams({{"maxVal",c->Int()}});

  TypeGen* counterTypeGen = global->newTypeGen(
    "CounterTypeGen",
    counterParams, //Params required for typegen
    [](Context* c, Values args) { //lambda for generating the type

      uint maxVal = args.at("maxVal")->get<int>();;
      uint width = bitsNeededToStore(maxVal);
      return c->Record({
        {"en",c->BitIn()}, 
        {"out",c->Array(width,c->Bit())},
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
  counter->setGeneratorDefFromFun([](Context* c, Values args, ModuleDef* def) {
  //counter->setGeneratorDefFromFun([](ModuleDef* def,Context* c, Type* t, Values args) {
    //ModuleDef* def : The circuit you are defining.
    //Type* t: The generated Type of the counter (Using your counterTypeGen function!)
    //Values args: The arguments supplied to the instance of the counter.
    
    //Similar to the typegen, lets extract the width;
    uint maxVal = args.at("maxVal")->get<int>();
    uint width = bitsNeededToStore(maxVal);      

    cout << "width = " << width << endl;

    Values wArg({{"width", Const::make(c, width)}});
    def->addInstance("ai","coreir.add",wArg);
    def->addInstance("ci","coreir.const",wArg,{{"value", Const::make(c, BitVector(width, 1))}});
    def->addInstance("ri","coreir.reg",{{"width", Const::make(c, width)},{"en", Const::make(c, true)}});
    def->addInstance("maxVal", "coreir.const", wArg, {{"value", Const::make(c, BitVector(width, maxVal))}});

    def->addInstance("zro", "coreir.const", wArg, {{"value", Const::make(c, BitVector(width, 0))}});
    def->addInstance("incMux", "coreir.mux", wArg);
    def->addInstance("eqMax", "coreir.eq", wArg);

    //Connections
    def->connect("self.clk", "ri.clk");
    def->connect("self.en", "ri.en");

    def->connect("ci.out", "ai.in0");
    def->connect("ri.out", "ai.in1");
    
    // Connect up mux to reg
    def->connect("ai.out", "incMux.in0");
    def->connect("zro.out", "incMux.in1");
    def->connect("eqMax.out", "incMux.sel");
    def->connect("incMux.out", "ri.in");

    // Create eq test
    def->connect("ri.out", "eqMax.in0");
    def->connect("maxVal.out", "eqMax.in1");

    // Register output
    def->connect("ri.out", "self.out");
  });
  
}


