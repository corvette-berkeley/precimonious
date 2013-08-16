#include "FunctionChange.hpp"

#include <llvm/DerivedTypes.h>
#include <llvm/Module.h>

using namespace std;
using namespace llvm;

FunctionChange::FunctionChange(Types aType, Value* aValue, string aSwit) : Change(aType, aValue) {
  swit = aSwit;
}

string FunctionChange::getSwitch() {
  return swit;
}
