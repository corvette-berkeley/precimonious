#include "Operators.hpp"
#include "ParseConfig.hpp"

#include <llvm/Module.h>
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <llvm/Support/raw_ostream.h>

void Operators::doInitialization(Module &module) {
	
	debugInfo.processModule(module);

	ParseConfig &parseConfig = getAnalysis<ParseConfig>();
	changes = parseConfig.getChanges();

	return;
}

bool Operators::runOnModule(Module &module) {

	doInitialization(module);

	// iterate through operator set
	
	vector<Change*>::iterator it;
	for (it = changes[OP].begin(); it != changes[OP].end(); it++) {
		Change *change = *it;
		Value *target = change->getValue();
		Type *new_type = change->getType().at(0);

		if (target) {
			if (FCmpInst* old_target = dyn_cast<FCmpInst>(target)) {
				changeFCmpInst(old_target, new_type);
			} else if (BinaryOperator* old_target = dyn_cast<BinaryOperator>(target)) {
				changeBinaryOperator(old_target, new_type);
			}
		}
	}

	return true;
}

void Operators::changeBinaryOperator(BinaryOperator* old_target, Type* new_type) {
	Value *val_1 = old_target->getOperand(0);
	Value *val_2 = old_target->getOperand(1);
	Type* old_type = val_1->getType();

	errs() << "Changing the precision of operator \"" << old_target->getOpcodeName() << "\" from " << *old_type << " to " << *new_type << ".\n";

	if (old_type->getTypeID() != new_type->getTypeID()) {
		if (new_type->getTypeID() > old_type->getTypeID()) {
			FPExtInst *ext_1 = new FPExtInst(val_1, new_type, "", old_target);
			FPExtInst *ext_2 = new FPExtInst(val_2, new_type, "", old_target);
			BinaryOperator* new_target = BinaryOperator::Create(old_target->getOpcode(), ext_1, ext_2, "", old_target);
			FPTruncInst *trunc = new FPTruncInst(new_target, old_type, "", old_target);
			old_target->replaceAllUsesWith(trunc);
		} else {
			FPTruncInst *trunc_1 = new FPTruncInst(val_1, new_type, "", old_target);
			FPTruncInst *trunc_2 = new FPTruncInst(val_2, new_type, "", old_target);
			BinaryOperator* new_target = BinaryOperator::Create(old_target->getOpcode(), trunc_1, trunc_2, "", old_target);
			FPExtInst *ext = new FPExtInst(new_target, old_type, "", old_target);
			old_target->replaceAllUsesWith(ext);
		}
		old_target->eraseFromParent();
	} else {
		errs() << "\tNo changes required.\n";  
	}

	return;
}

void Operators::changeFCmpInst(FCmpInst* old_target, Type* new_type) {
	Value *val_1 = old_target->getOperand(0);
	Value *val_2 = old_target->getOperand(1);
	Type* old_type = val_1->getType();

	errs() << "Changing the precision of operator \"" << old_target->getOpcodeName() << "\" from " << *old_type << " to " << *new_type << ".\n";

	if (old_type->getTypeID() != new_type->getTypeID()) {
		if (new_type->getTypeID() > old_type->getTypeID()) {
			FPExtInst *ext_1 = new FPExtInst(val_1, new_type, "", old_target);
			FPExtInst *ext_2 = new FPExtInst(val_2, new_type, "", old_target);
			FCmpInst* new_target = new FCmpInst(old_target, old_target->getPredicate(), ext_1, ext_2, "");
			old_target->replaceAllUsesWith(new_target);
		} else {
			FPTruncInst *trunc_1 = new FPTruncInst(val_1, new_type, "", old_target);
			FPTruncInst *trunc_2 = new FPTruncInst(val_2, new_type, "", old_target);
			FCmpInst* new_target = new FCmpInst(old_target, old_target->getPredicate(), trunc_1, trunc_2, "");
			old_target->replaceAllUsesWith(new_target);
		}
		old_target->eraseFromParent();
	} else {
		errs() << "\tNo changes required.\n";  
	}

	return;
}

void Operators::getAnalysisUsage(AnalysisUsage &AU) const {
	AU.addRequired<ParseConfig>();
	return;
}

char Operators::ID = 0;
static const RegisterPass<Operators> registration("operators", "Change the precision of operators");
