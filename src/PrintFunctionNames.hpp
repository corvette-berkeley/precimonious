#ifndef PRINT_FUNCTION_NAMES_GUARD
#define PRINT_FUNCTION_NAMES_GUARD 1

#include <llvm/Pass.h>

using namespace std;
using namespace llvm;

class PrintFunctionNames : public ModulePass {
  
public:
  PrintFunctionNames() : ModulePass(ID) {}
  
  virtual bool runOnModule(Module &M);
  
  virtual void getAnalysisUsage(AnalysisUsage &AU) const;

  static char ID; // Pass identification, replacement for typeid

};

#endif // PRINT_FUNCTION_NAMES_GUARD
