#ifndef FUNCTION_CALLS_GUARD
#define FUNCTION_CALLS_GUARD 1

#include "ParseConfig.hpp"

#include <llvm/Analysis/DebugInfo.h>
#include <llvm/Pass.h>

namespace llvm {
  class CallInst;
}

using namespace std;
using namespace llvm;

class FunctionCalls : public ModulePass {
  
public:
  FunctionCalls() : ModulePass(ID) {}

  void doInitialization(Module &module);  

  CallInst* changeFunctionCall(Module &module, Change* change);

  //bool runOnFunction(Module &module, Function &function);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const;

  virtual bool runOnModule(Module &module);

  static char ID; // Pass identification, replacement for typeid

private:
  DebugInfoFinder debugInfo;  

  map<ChangeType, Changes> changes;

};

#endif // FUNCTION_CALLS_GUARD
