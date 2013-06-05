#include "PrintVariableNames.hpp"

#include <llvm/Function.h>
#include <llvm/Instructions.h>
#include <llvm/ValueSymbolTable.h>
#include <llvm/ADT/ValueMap.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>

#include <iostream>
#include <sstream>

using namespace std;
using namespace llvm;

bool PrintVariableNames::runOnFunction(Function &F) {
	errs() << F.getName() << "\n";

	// print all symbols
	ValueSymbolTable &S = F.getValueSymbolTable();
	for (ValueSymbolTable::iterator v = S.begin(), ve = S.end(); v != ve; v++) {
		errs() << "\tvar: " << v->getKey() << "\n";
	} 

	BasicBlock &B = F.getEntryBlock();
	for (BasicBlock::iterator i = B.begin(), ie = B.end(); i != ie; i++) {
		Instruction *inst = i;
		// errs () << "\ttype: " << inst->getOpcodeName() << "\n";
		if (AllocaInst::classof(inst)) {
			AllocaInst *ainst = (AllocaInst *) inst;
			Type *type = ainst->getAllocatedType();
			errs () << "\talloca: " << ainst->getName() << type->getTypeID() << "\n";
		} else if (LoadInst::classof(inst)) {
			LoadInst *linst = (LoadInst *) inst;
			errs () << "\tload: " << linst->getPointerOperand()->getName() << "\n";
		}
	}

	return false;
}

void PrintVariableNames::getAnalysisUsage(AnalysisUsage &AU) const {
	AU.setPreservesAll();
}

char PrintVariableNames::ID = 1;
static const RegisterPass<PrintVariableNames> registration("print-vars", "Printing variable names");

