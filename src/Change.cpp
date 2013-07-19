#include "Change.hpp"

#include <llvm/DerivedTypes.h>
#include <llvm/Module.h>

using namespace std;
using namespace llvm;

Change::Change(Types aType, Value* aValue) {
  type = aType;
  value = aValue;
  field = -1;
}

Change::Change(Types aType, Value* aValue, int aField) {
  type = aType;
  value = aValue;
  field = aField;
}

Value* Change::getValue() {
  return value;
}

Types Change::getType() {
  return type;
}

int Change::getField() {
  return field;
}
