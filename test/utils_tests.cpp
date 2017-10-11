#include "catch.hpp"

#include "utils.h"

#include "coreir/simulator/interpreter.h"
#include "coreir/passes/transform/rungenerators.h"

using namespace std;
using namespace CoreIR;
using namespace CoreIR::Passes;

TEST_CASE("Off power of 2 counter") {
  Context* c = newContext();
  Namespace* global = c->getGlobal();

  addCounter(c, global);

  uint maxVal = 17;
  uint pcWidth = bitsNeededToStore(maxVal);

  Type* counterTestType =
    c->Record({
	{"en", c->BitIn()},
	  {"clk", c->Named("coreir.clkIn")},
	    {"counterOut", c->Array(pcWidth, c->Bit())}});

  Module* counterTest = global->newModuleDecl("counterMod", counterTestType);
  ModuleDef* def = counterTest->newModuleDef();

  def->addInstance("counter", "global.counter", {{"maxVal", Const::make(c, maxVal)}});

  def->connect("self.en", "counter.en");
  def->connect("self.clk", "counter.clk");
  def->connect("counter.out", "self.counterOut");

  counterTest->setDef(def);

  RunGenerators rg;
  rg.runOnNamespace(global);

  // Inline increment
  inlineInstance(def->getInstances()["counter"]);

  SimulatorState state(counterTest);
  state.setValue("counter$ri.out", BitVec(pcWidth, 10));
  state.setValue("self.en", BitVec(1, 1));
  state.setClock("self.clk", 0, 1);

  SECTION("After first step output is the initial register value") {

    state.execute();

    REQUIRE(state.getBitVec("self.counterOut") == BitVec(pcWidth, 10));
  }

  SECTION("7 full clock cycles after eleven there is a reset") {
    state.setValue("counter$ri.out", BitVec(pcWidth, 11));
    state.setValue("self.en", BitVec(1, 1));
    state.setClock("self.clk", 0, 1);
    state.setMainClock("self.clk");

    //cout << "Start  = " << state.getBitVec("self.counterOut") << endl;

    for (int i = 0; i < 14; i++) {
      state.execute();
      state.stepMainClock();
      cout << "Output = " << state.getBitVec("self.counterOut") << endl;
      cout << "ri     = " << state.getBitVec("counter$ri.out") << endl;    
    }


    REQUIRE(state.getBitVec("self.counterOut") == BitVec(pcWidth, 0));
  }
  
}

TEST_CASE("Incrementer") {
  Context* c = newContext();
  Namespace* global = c->getGlobal();

  addIncrementer(c, global);

  uint pcWidth = 3;

  Type* incTestType =
    c->Record({{"incIn", c->Array(pcWidth, c->BitIn())},
	{"incOut", c->Array(pcWidth, c->Bit())}});

  Module* incTest = global->newModuleDecl("incMod", incTestType);
  ModuleDef* def = incTest->newModuleDef();


  def->addInstance("incrementer", "global.inc", {{"width", Const::make(c, pcWidth)}});

  def->connect("self.incIn", "incrementer.in");
  def->connect("incrementer.out", "self.incOut");

  incTest->setDef(def);

  RunGenerators rg;
  rg.runOnNamespace(global);

  // Inline increment
  inlineInstance(def->getInstances()["incrementer"]);

  SimulatorState state(incTest);
  state.setValue("self.incIn", BitVec(pcWidth, 0));

  state.execute();

  REQUIRE(state.getBitVec("self.incOut") == BitVec(pcWidth, 1));

  state.setValue("self.incIn", BitVec(pcWidth, (1 << 2) | (1 << 1) | 1));

  state.execute();

  cout << "Output = " << state.getBitVec("self.incOut") << endl;

  REQUIRE(state.getBitVec("self.incOut") == BitVec(pcWidth, 0));

}

TEST_CASE("Increment or reset") {
  Context* c = newContext();
  Namespace* global = c->getGlobal();

  addIncrementer(c, global);
  addIncReset(c, global);

  uint width = 62;

  Type* irType =
    c->Record({
	{"in", c->Array(width, c->BitIn())},
	  {"out", c->Array(width, c->Bit())},
	    {"reset", c->BitIn()}
      });

  Module* irM = global->newModuleDecl("irM", irType);
  ModuleDef* def = irM->newModuleDef();

  def->addInstance("ir", "global.incReset", {{"width", Const::make(c, width)}});

  def->connect("self.in", "ir.in");
  def->connect("self.reset", "ir.selectBit");
  def->connect("ir.out", "self.out");

  irM->setDef(def);

  RunGenerators rg;
  rg.runOnNamespace(global);

  // Inline increment
  inlineInstance(def->getInstances()["ir"]);
  inlineInstance(def->getInstances()["ir$incrementer"]);

  cout << "Checking saving and loading pregen" << endl;
  if (!saveToFile(global, "incReset.json", irM)) {
    cout << "Could not save to json!!" << endl;
    c->die();
  }

  SimulatorState state(irM);

  SECTION("Reset") {
    state.setValue("self.in", BitVec(width, 23));
    state.setValue("self.reset", BitVec(1, 1));

    state.execute();

    REQUIRE(state.getBitVec("self.out") == BitVec(width, 0));

  }

  SECTION("No reset") {
    state.setValue("self.in", BitVec(width, 236));
    state.setValue("self.reset", BitVec(1, 0));

    state.execute();

    REQUIRE(state.getBitVec("self.out") == BitVec(width, 237));

  }

}
