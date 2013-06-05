#ifndef CHANGE_PRECISION_GUARD
#define CHANGE_PRECISION_GUARD 1

#include <llvm/Analysis/DebugInfo.h>
#include <llvm/Pass.h>

#include <map>
#include <vector>

namespace llvm {
  class Value;
}

using namespace std;
using namespace llvm;

typedef vector<Value*> Variables;

class ChangePrecision : public ModulePass {
  
public:
  ChangePrecision() : ModulePass(ID) {}
  
  bool doInitialization(Module &M);

  bool doFinalization(Module &M);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const;

  virtual bool runOnModule(Module &M);

  static char ID; // Pass identification, replacement for typeid

private:
  DebugInfoFinder debugInfo;  

  map<const Function*, Variables> locals;

};

#endif // CHANGE_PRECISION_GUARD
