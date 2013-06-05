#ifndef TRANSFORMER_GUARD
#define TRANSFORMER_GUARD 1

#include <llvm/Instructions.h>
#include "Debug.hpp"

namespace llvm {
  class Value;
  class ConstantInt;
}

using namespace std;
using namespace llvm;

class Transformer {
public: 
  static bool transform(value_use_iterator<User>, Value*, Value*, Type*, Type*, unsigned);
  static bool transform(LoadInst&, Value*, Value*, Type*, Type*, unsigned);
  static bool transform(StoreInst&, Value*, Value*, Type*, Type*, unsigned);
  static bool transform(BitCastInst&, Value*, Value*, Type*, Type*, unsigned);
  static bool transform(GetElementPtrInst&, Value*, Value*, Type*, Type*, unsigned);
  static bool transform(CallInst&, Value*, Value*, Type*, Type*, unsigned);
  static ConstantInt* getInt32(int n);
  static ConstantInt* getInt64(int n);
};

#endif
