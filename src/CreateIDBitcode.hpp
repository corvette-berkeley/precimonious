#ifndef CREATE_ID_BITCODE
#define CREATE_ID_BITCODE

#include <llvm/Pass.h>

using namespace std;
using namespace llvm;

class CreateIDBitcode : public ModulePass {
	
public:
  CreateIDBitcode() : ModulePass(ID)  {}

  virtual bool runOnModule(Module &M);
  
  bool runOnFunction(Function &F);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const;

	static int instId; // Instruction id

  static char ID; // Pass identification, replacement for typeid
};

#endif // CREATE_ID_BITCODE
