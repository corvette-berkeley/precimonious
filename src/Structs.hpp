#ifndef STRUCTS_GUARD
#define STRUCTS_GUARD 1

#include <llvm/Analysis/DebugInfo.h>
#include <llvm/Pass.h>

#include <map>
#include <vector>

namespace llvm {
  class StructType;
}

using namespace std;
using namespace llvm;

class Structs : public ModulePass {
  
public:
  Structs() : ModulePass(ID) {}

  void doInitialization(Module &module);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const;

  virtual bool runOnModule(Module &module);

  bool runOnFunction(Function &function);

  void printGlobals(Module &module);

  void printLocals(Function &function);

  void printStructFields(StructType *structType);

  void changeLocal(Value *value, LLVMContext& context);

  static char ID; // Pass identification, replacement for typeid

private:
  DebugInfoFinder debugInfo;  

  vector<Value*> locals;

  vector<Value*> globals;
};

#endif // STRUCTS_GUARD

