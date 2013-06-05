#include "Instrument.hpp"

#include <llvm/Module.h>
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>

#include <fstream>


void Instrument::doInitialization(Module &module) {	
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
  
  return;
}


bool Instrument::runOnModule(Module &module) {
  doInitialization(module);

  // iterate through functions in module
  for(Module::iterator f = module.begin(), fe = module.end(); f != fe; f++) {
    runOnFunction(module, *f);
  }

  return true;
}


static void createCallInst(Module &module, Instruction *instruction, Type *type, string prefixName) {

  Function *callee = NULL;

  if (type) {
    unsigned int typeID = type->getTypeID();
    
    switch(typeID) {
    case Type::FloatTyID:
      callee = module.getFunction(prefixName + "Float");
      break;
      
    case Type::DoubleTyID:
      callee = module.getFunction(prefixName + "Double");
      break;
      
    case Type::X86_FP80TyID:
      callee = module.getFunction(prefixName + "LongDouble");
      break;      
    }
  }
  else {
    callee = module.getFunction(prefixName);
  }

  
  if (callee) {
    CallInst::Create(callee, "", instruction);
  }

  return;
}


bool Instrument::runOnFunction(Module &module, Function &function) {

  if (! function.isDeclaration() && (excludedFunctions.find(function.getName()) == excludedFunctions.end())) {
    for(Function::iterator b = function.begin(), be = function.end(); b != be; b++) {
      for(BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {

	if (StoreInst *storeInst = dyn_cast<StoreInst>(i)) {
	  Type *type = storeInst->getOperand(0)->getType();
	  string prefixName = "incrementStoreInst";
	  createCallInst(module, i, type, prefixName);
	}

	if (LoadInst *loadInst = dyn_cast<LoadInst>(i)) {
	  Value *pointerOperand = loadInst->getPointerOperand();
	  PointerType* pointerType = dyn_cast<PointerType>(pointerOperand->getType());
	  Type *type = pointerType->getElementType();
	  string prefixName = "incrementLoadInst";
	  createCallInst(module, i, type, prefixName);
	}

	if (BinaryOperator *binOp = dyn_cast<BinaryOperator>(i)) {
	  Type *type = binOp->getOperand(0)->getType();	  
	  switch(binOp->getOpcode()) {
	  case Instruction::FAdd:
	  case Instruction::FSub:
	  case Instruction::FMul:
	  case Instruction::FDiv: {	    
	    string prefixName = "incrementArithOpInst";
	    createCallInst(module, i, type, prefixName);
	    break;
	  }
	  default:
	    // do nothing
	    break;
	  }
	}
	  
	if (FCmpInst *cmpOp = dyn_cast<FCmpInst>(i)) {
	  Type *type = cmpOp->getOperand(0)->getType();
	  string prefixName = "incrementCmpOpInst";
	  createCallInst(module, i, type, prefixName);
	}

	if (dyn_cast<FPTruncInst>(i)) {
	  string prefixName = "incrementFPTruncInst";
	  createCallInst(module, i, NULL, prefixName);
	}

	if (dyn_cast<FPExtInst>(i)) {
	  string prefixName = "incrementFPExtInst";
	  createCallInst(module, i, NULL, prefixName);
	}

      } // for BasicBlock
    } // for Function
  } // if

  if (function.getName() == "main") {
    for(Function::iterator b = function.begin(), be = function.end(); b != be; b++) {
      for(BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {
        if (ReturnInst *returnInst = dyn_cast<ReturnInst>(i)) {
          if (function.getName() == "main") {
            if (Function *callee = module.getFunction("printCounters")) {
              CallInst::Create(callee, "", returnInst);
            }
          }
        }

      }
    }

  }
  return true;
}


void Instrument::getAnalysisUsage(AnalysisUsage &AU __attribute__((unused))) const {
  return;
}


char Instrument::ID = 0;
static const RegisterPass<Instrument> registration("instrument", "Instruments FP Stores, Loads, Arithmetic Operators and Comparison Operators");
