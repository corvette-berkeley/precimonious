#include "FunctionCalls.hpp"
#include "FunctionChange.hpp"
#include "ParseConfig.hpp"
#include "Variables.hpp"

#include <llvm/Analysis/DebugInfo.h>
#include <llvm/Instruction.h>
#include <llvm/Instructions.h>
#include <llvm/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ValueSymbolTable.h>

#include <iostream>
#include <sstream>

using namespace std;
using namespace llvm;


void FunctionCalls::doInitialization(Module &module) {

  debugInfo.processModule(module);
  ParseConfig &parseConfig = getAnalysis<ParseConfig>();
  changes = parseConfig.getChanges();

  return;
}


bool FunctionCalls::runOnModule(Module &module) {
  errs() << "Replacing function calls\n";
  doInitialization(module);

  vector<Change*>::iterator it;
  for(it = changes[CALL].begin(); it != changes[CALL].end(); it++) {
    changeFunctionCall(module, *it);
  }
  return true;
}  


CallInst* FunctionCalls::changeFunctionCall(Module &module, Change* change) {
  FunctionChange *funChange = (FunctionChange*)change;
  CallInst *oldCallInst = dyn_cast<CallInst>(funChange->getValue());
  CallInst *newCallInst = NULL;
  string oldFunction = oldCallInst->getCalledFunction()->getName();
  string newFunction = funChange->getSwitch();

  // TODO: use the types vector to not assume signature
  Function *newCallee = module.getFunction(newFunction);
  if (newCallee) {
    errs() << "Changing function call from " << oldFunction << " to " << newFunction << "\n";
    
    if (oldFunction != newFunction) {
      // retrieving original operand
      Value *oldOperand = oldCallInst->getOperand(0);
      
      // downcasting operand    
      Type *fType = Type::getFloatTy(module.getContext());
      FPTruncInst *newOperand = new FPTruncInst(oldOperand, fType, "", oldCallInst);
      
      // populating array of operands
      vector<Value*> operands;
      operands.push_back(newOperand);
      ArrayRef<Value*> *arrayRefOperands = new ArrayRef<Value*>(operands);
      
      // creating the new CallInst
      newCallInst = CallInst::Create(newCallee, *arrayRefOperands, "newCall", oldCallInst);
      
      // casting result to double
      Type *dType = Type::getDoubleTy(module.getContext());
      FPExtInst *result = new FPExtInst(newCallInst, dType, "", oldCallInst);
      
      // replacing all uses of call instruction
      oldCallInst->replaceAllUsesWith(result);
    
      // deleting old callInst
      oldCallInst->eraseFromParent();
      
      errs() << "\tChange was successful\n";
    }
    else {
      errs() << "\tNo change required\n";
    }
  }
  else {
    errs() << "\tDid not find function " << newFunction << "\n";
  }
  
  return newCallInst;
}


void FunctionCalls::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<ParseConfig>();
  AU.addRequired<Variables>();
}


char FunctionCalls::ID = 0;
static const RegisterPass<FunctionCalls> registration("function-calls", "Replaces function calls");
