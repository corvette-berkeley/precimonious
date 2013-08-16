#include "StrChange.hpp"
#include "FuncStrChange.hpp"

#include <llvm/DerivedTypes.h>
#include <llvm/Module.h>

using namespace std;
using namespace llvm;

FuncStrChange::FuncStrChange(string aClassification, string aTypes, int aField, string aSwit) : StrChange(aClassification, aTypes, aField) {
  swit = aSwit;
}

string FuncStrChange::getSwitch() {
  return swit;
}
