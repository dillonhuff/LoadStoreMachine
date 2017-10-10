#define CATCH_CONFIG_MAIN

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
