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
  if (!loadFromFile(c,"./test/sim_ready_conv_3_1.json")) {
    cout << "Could not Load from json!!" << endl;
    c->die();
  }

  Module* m = global->getModule("DesignTop");

  assert(m != nullptr);

  SimulatorState state(m);
  state.setValue("self.in_0", BitVector(16, "0000000000000000"));
  state.setMainClock("self.clk");

  state.setClock("self.clk", 0, 1);

  state.execute();

  // state.execute();

  // cout << "out = " << state.getBitVec("self.out") << endl;
  
}
