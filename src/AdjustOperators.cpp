#include "AdjustOperators.hpp"
#include "Variables.hpp"
#include "FunctionCalls.hpp"

#include <llvm/Module.h>
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>

#include <fstream>

void AdjustOperators::doInitialization(Module &module) {	
  debugInfo.processModule(module);

  ifstream inFile(ExcludedFunctionsFileName.c_str());
  string name;
  
  if (!inFile) {
    errs() << "Unable to open " << ExcludedFunctionsFileName << '\n';
    exit(1);
  }
  
  while(inFile >> name) {
    excludedFunctions.insert(name);
  }
  inFile.close();
  
  inFile.open (IncludedFunctionsFileName.c_str(), ifstream::in);
  if (!inFile) {
    errs() << "Unable to open " << IncludedFunctionsFileName << '\n';
    exit(1);
  }
  
  while(inFile >> name) {
    includedFunctions.insert(name);
  }
  inFile.close();

  return;
}


bool AdjustOperators::runOnModule(Module &module) {
  errs() << "Running AdjustOperators\n";
  doInitialization(module);

  // iterate through operator set
  for(Module::iterator f = module.begin(), fe = module.end(); f != fe; f++) {
    runOnFunction(*f);
  }
  errs() << "Done!\n";
  return true;
}


bool AdjustOperators::runOnFunction(Function &function) {
  string name = function.getName();
  
  if (! function.isDeclaration()  && (includedFunctions.find(name) != includedFunctions.end()) && (excludedFunctions.find(name) == excludedFunctions.end())) {
    errs() << "=== Exploring function: " << function.getName() << "\n";
    for(Function::iterator b = function.begin(), be = function.end(); b != be; b++) {
      for(BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {
	if (BinaryOperator* binOp = dyn_cast<BinaryOperator>(i)) {
	  switch(binOp->getOpcode()) {
	  case Instruction::FAdd:
	  case Instruction::FSub:
	  case Instruction::FMul:
	  case Instruction::FDiv:
	    if ((dyn_cast<CastInst>(binOp->getOperand(0)) && (dyn_cast<SIToFPInst>(binOp->getOperand(1)))) ||
	        (dyn_cast<CastInst>(binOp->getOperand(1)) && (dyn_cast<SIToFPInst>(binOp->getOperand(0))))) {
	      changeBinaryOperatorInt(binOp);
	    }
	    else {
	      changeBinaryOperator(binOp);
	    }
	    break;
	  default:
	    // do nothing
	    break;
	  }
	}
	else if (FCmpInst* cmpOp = dyn_cast<FCmpInst>(i)) {
	  changeFCmpInst(cmpOp);
	}
      }
    }
  }
  clean();
  return true;
}


void AdjustOperators::clean() {
  for(unsigned i = 0; i < erase.size(); i++) {
    erase[i]->eraseFromParent();
  }
  erase.clear();
  return;
}


void AdjustOperators::changeBinaryOperatorInt(BinaryOperator* oldTarget) {
  
  Value *val1 = oldTarget->getOperand(0);
  Value *val2 = oldTarget->getOperand(1);

  SIToFPInst *sit = NULL;
  CastInst *cast = NULL;
  if (((sit = dyn_cast<SIToFPInst>(val1)) && (cast = dyn_cast<CastInst>(val2))) ||
      ((sit = dyn_cast<SIToFPInst>(val2)) && (cast = dyn_cast<CastInst>(val1)))) {
    sit = new SIToFPInst(sit->getOperand(0), cast->getOperand(0)->getType(), "", oldTarget);
  }

  // create new operator
  BinaryOperator *newTarget = BinaryOperator::Create(oldTarget->getOpcode(), cast->getOperand(0), sit, "", oldTarget);
    
  // replace all uses
  if (newTarget->getType()->getTypeID() < oldTarget->getType()->getTypeID()) {
    FPExtInst *ext = new FPExtInst(newTarget, oldTarget->getType(), "", oldTarget);
    oldTarget->replaceAllUsesWith(ext);
  }
  else {
    oldTarget->replaceAllUsesWith(newTarget);
  }
  
#ifdef DEBUG
  errs() << "Changing operator:\n";
  oldTarget->dump();
  newTarget->dump();
  erase.push_back(oldTarget);
  errs() << "Done\n";
#endif

  return;
}


void AdjustOperators::changeBinaryOperator(BinaryOperator* oldTarget) {
  
  Value *val1 = oldTarget->getOperand(0);
  Value *val2 = oldTarget->getOperand(1);

  CastInst *cast1 = NULL;
  if ((cast1 = dyn_cast<FPExtInst>(val1)) || (cast1 = dyn_cast<FPTruncInst>(val1))) {
    val1 = cast1->getOperand(0);
  }

  CastInst *cast2 = NULL;
  if ((cast2 = dyn_cast<FPExtInst>(val2)) || (cast2 = dyn_cast<FPTruncInst>(val2))) {
    val2 = cast2->getOperand(0);
  }

  if ((cast1 == NULL && cast2 == NULL) ||
      (val1->getType()->getTypeID() == oldTarget->getType()->getTypeID()) ||
      (val2->getType()->getTypeID() == oldTarget->getType()->getTypeID())) {
    // no operator adjustment to make
    return;
  }
    
  // adjust either operand, if needed
  if (val1->getType()->getTypeID() > val2->getType()->getTypeID()) {
    val2 = CastInst::CreateFPCast(val2, val1->getType(), "", oldTarget);
  }
  else if (val1->getType()->getTypeID() < val2->getType()->getTypeID()) {
    val1 = CastInst::CreateFPCast(val1, val2->getType(), "", oldTarget);
  }
  
  // create new operator
  BinaryOperator *newTarget = BinaryOperator::Create(oldTarget->getOpcode(), val1, val2, "", oldTarget);
    
  // replace all uses
  if (newTarget->getType()->getTypeID() < oldTarget->getType()->getTypeID()) {
    FPExtInst *ext = new FPExtInst(newTarget, oldTarget->getType(), "", oldTarget);
    oldTarget->replaceAllUsesWith(ext);
  }
  else {
    oldTarget->replaceAllUsesWith(newTarget);
  }
  
#ifdef DEBUG
  errs() << "Changed operator:\n";
  oldTarget->dump();
  oldTarget->getOperand(0)->dump();
  newTarget->dump();
  newTarget->getOperand(0)->dump();
  erase.push_back(oldTarget);
  errs() << "Done\n";
#endif

  return;
}


void AdjustOperators::changeFCmpInst(FCmpInst* oldTarget) {
  Value *val1 = oldTarget->getOperand(0);
  Value *val2 = oldTarget->getOperand(1);

  CastInst *cast1 = NULL;
  if ((cast1 = dyn_cast<FPExtInst>(val1)) || (cast1 = dyn_cast<FPTruncInst>(val1))) {
    val1 = cast1->getOperand(0);
  }

  CastInst *cast2 = NULL;
  if ((cast2 = dyn_cast<FPExtInst>(val2)) || (cast2 = dyn_cast<FPTruncInst>(val2))) {
    val2 = cast2->getOperand(0);
  }

  if ((cast1 == NULL && cast2 == NULL) || 
      (val1->getType()->getTypeID() == oldTarget->getType()->getTypeID()) ||
      (val2->getType()->getTypeID() == oldTarget->getType()->getTypeID())) {
    // no operator adjustment to make
    return;
  }
    
  // adjust either operand, if needed
  if (val1->getType()->getTypeID() > val2->getType()->getTypeID()) {
    val2 = CastInst::CreateFPCast(val2, val1->getType(), "", oldTarget);
  }
  else if (val1->getType()->getTypeID() < val2->getType()->getTypeID()) {
    val1 = CastInst::CreateFPCast(val1, val2->getType(), "", oldTarget);
  }
  
  // create new operator
  FCmpInst* newTarget = new FCmpInst(oldTarget, oldTarget->getPredicate(), val1, val2, "");
    
  // replace all uses
  if (newTarget->getType()->getTypeID() < oldTarget->getType()->getTypeID()) {
    FPExtInst *ext = new FPExtInst(newTarget, oldTarget->getType(), "", oldTarget);
    oldTarget->replaceAllUsesWith(ext);
  }
  else {
    oldTarget->replaceAllUsesWith(newTarget);
  }
  
#ifdef DEBUG
  errs() << "Changed operator:\n";
  oldTarget->dump();
  oldTarget->getOperand(0)->dump();
  newTarget->dump();
  newTarget->getOperand(0)->dump();
  erase.push_back(oldTarget);
  errs() << "Done\n";
#endif

  return;
}


void AdjustOperators::getAnalysisUsage(AnalysisUsage &AU __attribute__((unused))) const {
  //AU.addRequired<Variables>();
  AU.addRequired<FunctionCalls>();
  return;
}


char AdjustOperators::ID = 0;
static const RegisterPass<AdjustOperators> registration("adjust-operators", "Adjusts the precision of operators depending on new types for operands");
