#define CATCH_CONFIG_MAIN

#include "coreir/libs/commonlib.h"
#include "coreir/simulator/interpreter.h"

#include "catch.hpp"

#include "programMem.h"

using namespace CoreIR;
//using namespace CoreIR::Passes;
using namespace std;


TEST_CASE("Program control test") {
  Context* c = newContext();
  Namespace* global = c->getGlobal();

  addProgramMem(c, global);
  
}

TEST_CASE("Whole architecture") {
  
}

TEST_CASE("Harris") {
  Context* c = newContext();
  Namespace* global = c->getGlobal();
  Namespace* common = CoreIRLoadLibrary_commonlib(c);

  cout << "loading" << endl;
  //if (!loadFromFile(c,"./test/sim_ready_conv_3_1.json")) {
  if (!loadFromFile(c,"./test/sim_ready_harris.json")) {
    cout << "Could not Load from json!!" << endl;
    c->die();
  }

  Module* m = global->getModule("DesignTop");

  assert(m != nullptr);

  SimulatorState state(m);
  state.setValue("self.in_0", BitVector(16, "0000000000000000"));
  state.setMainClock("self.clk");

  state.setClock("self.clk", 0, 1);

  BitVector one(16, "1");
  BitVector nextIn(16, "0");
  for (int i = 0; i < 200; i++) {
    state.setValue("self.in_0", nextIn);
    nextIn = add_general_width_bv(nextIn, one);
    state.execute();    

  }

  cout << "out = " << state.getBitVec("self.out") << endl;
  
}
