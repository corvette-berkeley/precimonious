#include "Change.hpp"

#include <llvm/DerivedTypes.h>
#include <llvm/Module.h>

using namespace std;
using namespace llvm;

Change::Change(Types aType, Value* aValue) {
	type = aType;
	value = aValue;
}

Value* Change::getValue() {
	return value;
}

Types Change::getType() {
	return type;
}
