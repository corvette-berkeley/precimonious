#include "MeasureMetric.hpp"

#include <llvm/Instructions.h>
#include <llvm/Module.h>
#include <llvm/Pass.h>
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <llvm/Support/raw_ostream.h>

#include <map>
#include <string>
#include <iostream>
#include <fstream>

void MeasureMetric::doInitialization(Module &M) {
	debugInfo.processModule(M);
	castInstCount = 0;
	score = 0;
	arithOp["float"] = 0;
	arithOp["double"] = 0;
	arithOp["longdouble"] = 0;
	cmpOp["float"] = 0;
	cmpOp["double"] = 0;
	cmpOp["longdouble"] = 0;
	loadOp["float"] = 0;
	loadOp["double"] = 0;
	loadOp["longdouble"] = 0;
	storeOp["float"] = 0;
	storeOp["double"] = 0;
	storeOp["longdouble"] = 0;

	return;
}

bool MeasureMetric::runOnModule(Module &M) {
	doInitialization(M);

	for(Module::iterator f = M.begin(), fe = M.end(); f != fe; f++) {
		if (!f->isDeclaration()) {
			runOnFunction(*f);
		}
	}

	doFinalization();

	return false;
}

bool MeasureMetric::runOnFunction(Function &F) {
	for (Function::iterator b = F.begin(), be = F.end(); b != be; b++) {
		for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {
			Instruction *inst = i;
			if (dyn_cast<CastInst>(inst)) {
				castInstCount++;
			} else if (FCmpInst* target = dyn_cast<FCmpInst>(inst)) {
				Value *val1 = target->getOperand(0);
				increaseCounter(val1->getType(), cmpOp);
			} else if (BinaryOperator *target = dyn_cast<BinaryOperator>(inst)) {
				Value *val1 = target->getOperand(0);
				increaseCounter(val1->getType(), arithOp);
			} else if (LoadInst *target = dyn_cast<LoadInst>(inst)) {
				Value *val1 = target->getPointerOperand();
				PointerType* pointerType = (PointerType*) val1->getType();
				increaseCounter(pointerType->getElementType(), loadOp);
			} else if (StoreInst* target = dyn_cast<StoreInst>(inst)) {
				Value *val1 = target->getOperand(0);
				increaseCounter(val1->getType(), storeOp);
			}
		}
	}

	return false;
}

void MeasureMetric::increaseCounter(Type* type, map<string, int> &counter) {
	if (type->isFloatTy()) {
		counter["float"]++;
		score = score + 32;
	} else if (type->isDoubleTy()) {
		counter["double"]++;
		score = score + 64;
	} else if (type->isX86_FP80Ty()) {
		counter["longdouble"]++;
		score = score + 80;
	}

	return;
}

void MeasureMetric::printCounter(map<string, int> &counter) {
	errs() << "\t float: " << counter["float"] << "\n";
	errs() << "\t double: " << counter["double"] << "\n";
	errs() << "\t X86_FP80: " << counter["longdouble"] << "\n";

	return;
}

void MeasureMetric::doFinalization() {
	errs() << "The number of casting instruction: " << castInstCount << "\n";
	errs() << "The number of arithmetic operations\n";
	printCounter(arithOp);
	errs() << "The number of comparison operations\n";
	printCounter(cmpOp);
	errs() << "The number of load instructions\n";
	printCounter(loadOp);
	errs() << "The number of store instructions\n";
	printCounter(storeOp);
	ofstream scoreFile;
	scoreFile.open("score.cov", ios_base::app);
	scoreFile << "#score\n";
	scoreFile << score << "\n";
	scoreFile.close();

	return;
}

void MeasureMetric::getAnalysisUsage(AnalysisUsage &AU) const {
	AU.setPreservesAll();
}

char MeasureMetric::ID = 0;

static const RegisterPass<MeasureMetric> registration("measure-metric", "Print software metric information");
