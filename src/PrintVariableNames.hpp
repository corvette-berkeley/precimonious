#ifndef PRINT_VARIABLE_NAMES_GUARD
#define PRINT_VARIABLE_NAMES_GUARD 1

#include <llvm/Pass.h>

using namespace std;
using namespace llvm;

class PrintVariableNames : public FunctionPass {

public:
	PrintVariableNames() : FunctionPass(ID) {} 

	virtual bool runOnFunction(Function &F);

	virtual void getAnalysisUsage(AnalysisUsage &AU) const;

	static char ID; // Pass identification, replacement for typeid

};

#endif // PRINT_VARIABLE_NAMES_GUARD
