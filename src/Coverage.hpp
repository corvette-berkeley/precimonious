#ifndef COVERAGE_GUARD
#define COVERGAGE_GUARD 1

#include <llvm/Analysis/DebugInfo.h>
#include <llvm/Instructions.h>
#include <llvm/Module.h>
#include <llvm/Pass.h>
#include <llvm/Support/CommandLine.h>

#include <set>

namespace llvm {
}

using namespace std;
using namespace llvm;
extern cl::opt<string> ExcludedFunctionsFileName;

class Coverage : public ModulePass {
public:
  Coverage() : ModulePass(ID) {}

  void doInitialization(Module &module);

  bool runOnFunction(Module &module, Function &function);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const;

  virtual bool runOnModule(Module &module);

  static char ID; // Pass identification, replacement for typeid

private:
  set<string> excludedFunctions;
  DebugInfoFinder debugInfo;
};

#endif
