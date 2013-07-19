#ifndef VARIABLES_GUARD
#define VARIABLES_GUARD 1

#include "ParseConfig.hpp"

#include <llvm/Analysis/DebugInfo.h>
#include <llvm/Pass.h>

#include <map>
#include <vector>

namespace llvm {
  class AllocaInst;
  class ConstantInt;
  class Value;
}

using namespace std;
using namespace llvm;

class Variables : public ModulePass {
  
public:
  Variables() : ModulePass(ID) {}

  void doInitialization(Module &module);

  void changeGlobal(Change* change, Module &module);

  AllocaInst* changeLocal(Change* change);

  AllocaInst* changeLocal(Value* value, Type* type);

  AllocaInst* changeLocal(Value* value, ArrayType* type);

  AllocaInst* changeLocal(Value* value, PointerType* type);

  AllocaInst* changeLocal(Value* value, StructType* type/*, int field*/);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const;

  virtual bool runOnModule(Module &module);

  static unsigned getAlignment(Type* type);
  
  static ConstantInt* getInt32(int n);

  static ConstantInt* getInt64(int n);

  static ConstantInt* getSizeInBits(Type *type);

  static ConstantInt* getAlignmentInBits(Type *type);

  static MDNode* getTypeMetadata(Module& module, DIVariable &oldDIVar, Type* newType);

  static void updateMetadata(Module& module, Value* oldTarget, Value* newTarget, Type* newType);

  static char ID; // Pass identification, replacement for typeid

private:
  DebugInfoFinder debugInfo;  

  map<ChangeType, Changes> changes;
};

#endif // VARIABLES_GUARD

