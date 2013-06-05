#ifndef REMOVE_DEAD_CASTING_GUARD
#define REMOVE_DEAD_CASTING_GUARD 1

#include <llvm/Analysis/DebugInfo.h>
#include <llvm/Pass.h>
#include <llvm/Support/CommandLine.h>

#include <set>

namespace llvm {
	class Value;
}

using namespace std;
using namespace llvm;

extern cl::opt<string> ExcludedFunctionsFileName;
extern cl::opt<string> IncludedFunctionsFileName;


class RemoveDeadCasting : public ModulePass {

public:
  RemoveDeadCasting() : ModulePass(ID) {} 
  
  void doInitialization(Module &M);
  
  virtual void getAnalysisUsage(AnalysisUsage &AU) const;
  
  virtual bool runOnModule(Module &M);

  bool runOnFunction(Function &F);
  
  static char ID; // Pass identification, replacement for typeid

private:
  DebugInfoFinder debugInfo;
  
  set<string> excludedFunctions;

  set<string> includedFunctions;
};

#endif
