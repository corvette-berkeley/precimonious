#ifndef ADJUST_OPERATORS_GUARD
#define ADJUST_OPERATORS_GUARD 1

#include <llvm/Analysis/DebugInfo.h>
#include <llvm/Instructions.h>
#include <llvm/Pass.h>
#include <llvm/Module.h>
#include <llvm/Support/CommandLine.h>

#include <vector>
#include <set>

namespace llvm {
	class Value;
	class BinaryOperator;
	class FCmpInst;
	class Type;
}

using namespace std;
using namespace llvm;
extern cl::opt<string> ExcludedFunctionsFileName;
extern cl::opt<string> IncludedFunctionsFileName;


class AdjustOperators : public ModulePass {
public:
  AdjustOperators() : ModulePass(ID) {}

  void doInitialization(Module &module);

  bool runOnFunction(Function &function);

  void clean();

  virtual void getAnalysisUsage(AnalysisUsage &AU) const;

  virtual bool runOnModule(Module &module);

  static char ID; // Pass identification, replacement for typeid

private:
  void changeBinaryOperator(BinaryOperator* old_target);

  void changeBinaryOperatorInt(BinaryOperator* old_target);

  void changeFCmpInst(FCmpInst* old_target);

  DebugInfoFinder debugInfo;  

  vector<Instruction*> erase;

  set<string> excludedFunctions;

  set<string> includedFunctions;
};

#endif
