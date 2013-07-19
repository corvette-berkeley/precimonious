
#include "Structs.hpp"
#include "Transformer.hpp"

#include <llvm/Analysis/DebugInfo.h>
#include <llvm/Analysis/DIBuilder.h>
#include <llvm/Constants.h>
#include <llvm/IntrinsicInst.h>
#include <llvm/DerivedTypes.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/ValueSymbolTable.h>
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <llvm/Support/raw_ostream.h>
#include <llvm/Analysis/Verifier.h>

#include <iostream>
#include <fstream>

void Structs::doInitialization(Module &module) {  
  debugInfo.processModule(module);
  return;
}


void Structs::printStructFields(StructType *structType) {

  for(StructType::element_iterator it = structType->element_begin(); it != structType->element_end(); it++) {
    errs() << "\t\t";
    (*it)->dump();
    errs() << "\n";
  }
  return;
}


void Structs::printGlobals(Module &module) {
  
  for (Module::global_iterator it = module.global_begin(); it != module.global_end(); it++) {
    Value *value = it;

    if (GlobalVariable *global = dyn_cast<GlobalVariable>(value)) {
      string name = global->getName();

      PointerType* pointerType = global->getType();      
      Type* elementType = pointerType->getElementType();

      if (StructType* structType = dyn_cast<StructType>(elementType)) {
	globals.push_back(value);
	errs() << "\tStruct: " << structType->getName() << "\n";
	printStructFields(structType);
      }
    }
  }
  return;
}


static Type* findLocalType(Value *value) {
  Type *type = NULL;
  if (AllocaInst* alloca = dyn_cast<AllocaInst>(value)) {
    type = alloca->getAllocatedType();
  }
  else if (Argument* arg = dyn_cast<Argument>(value)) {
    type = arg->getType();
  }
  return type;
}


void Structs::printLocals(Function &function) {

  const ValueSymbolTable& symbolTable = function.getValueSymbolTable();
  ValueSymbolTable::const_iterator it = symbolTable.begin();
  
  for(; it != symbolTable.end(); it++) {
    Value *value = it->second;
    Type *type = findLocalType(value);

    if (type) {
      if (StructType *structType = dyn_cast<StructType>(type)) {
	locals.push_back(value);
	errs() << "Local structure: " << structType->getName() << "\n";
	printStructFields(structType);
	errs() << "\t\t";
	structType->dump();
	errs() << "\n";
      }
    }
  }
  return;
}


void Structs::changeLocal(Value *value, LLVMContext &context) {

  errs() << "@changeLocal\n";
  Type *type = findLocalType(value);

  if (StructType *oldType = dyn_cast<StructType>(type)) {
    errs() << value->getName() << "\n";

    // Changing third field from double to float 
    // Creating new struct type based on old
    vector<Type*> fields;
    unsigned int fieldToChange = 2;
  
    errs() << "Number of elements: " << oldType->getNumElements() << "\n";

    for(unsigned int i = 0; i < oldType->getNumElements(); i++) {
      if (i != fieldToChange) {
	fields.push_back(oldType->getElementType(i));
      }
      else {
	// create new type
	// this case from double to float
	Type *newFieldType = Type::getFloatTy(context);
	fields.push_back(newFieldType);
      }
    }

    ArrayRef<Type*> *arrayRefFields = new ArrayRef<Type*>(fields);
    StructType *newType = StructType::create(*arrayRefFields, "hello");
    /////
    StringRef name = oldType->getName();
    newType->setName(name);

    errs() << "New struct type: \n";
    newType->dump();
    errs() << "\n";

    // Update function: create new var with new type
    // It seems to be the same as main changeLocal. Just get new AllocaInst
    AllocaInst* newTarget = NULL;
    vector<Instruction*> erase;

    if (AllocaInst *oldTarget = dyn_cast<AllocaInst>(value)) {

      errs() << "Changing the precision of variable \"" << oldTarget->getName() << "\" from " << *oldType 
	     << " to " << *newType << ".\n";
      
      unsigned alignment = oldTarget->getAlignment();

      newTarget = new AllocaInst(newType, 0, alignment, "new", oldTarget);
      newTarget->takeName(oldTarget);

      // iterating through instructions using old getelementptr instructions
      Value::use_iterator it = oldTarget->use_begin();
      for(; it != oldTarget->use_end(); it++) {
	
	errs() << "A use: \n";
	it->dump();
	errs() << "\n";

	if (GetElementPtrInst *inst = dyn_cast<GetElementPtrInst>(*it)) {
	  if (ConstantInt *constantInt = dyn_cast<ConstantInt>(inst->getOperand(2))) {
	    unsigned int index = constantInt->getLimitedValue();
	    
	    unsigned alignment = 4; // hard coded fornow
	    Type *newFieldType = Type::getFloatTy(context);
	    Type *oldFieldType = oldType->getElementType(index);
	    bool is_erased = Transformer::transform(it, newTarget, oldTarget, newFieldType, oldFieldType, alignment);
	    	    
	    if (!is_erased) {
	      erase.push_back(dyn_cast<Instruction>(*it));
	    }
	    
	    
	  }
	}
      }

      // erasing uses of old instructions
      for(unsigned int i = 0; i < erase.size(); i++) {
	erase[i]->eraseFromParent();
      }

    }
  }
  return;
}


bool Structs::runOnModule(Module &module) {
  errs() << "Running Structs\n";

  LLVMContext& context = module.getContext();
  doInitialization(module);
  printGlobals(module);

  for(Module::iterator f = module.begin(), fe = module.end(); f != fe; f++) {
    runOnFunction(*f);
  }

  // printing names of structs after collection
  for(vector<Value*>::iterator it = globals.begin(); it != globals.end(); it++) {
    errs() << "Global: " << (*it)->getName() << "\n";
  }

  for(vector<Value*>::iterator it = locals.begin(); it != locals.end(); it++) {
    errs() << "Local: " << (*it)->getName() << "\n";
  }
  
  // Searching for a given struct by name (by hand now)
  // TODO: change ParseConfig
  string name = "LS";
  for(vector<Value*>::iterator it = locals.begin(); it != locals.end(); it++) {
    if ((*it)->getName() == name) {
      errs() << "Found the variable!!\n";
      changeLocal(*it, context);
    }
  }

  return true;
}


bool Structs::runOnFunction(Function &function) {
  errs() << "Function: " << function.getName() << "\n";
  printLocals(function);
  return true;
}


void Structs::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
}


char Structs::ID = 0;
static const RegisterPass<Structs> registration("structs", "Change the precision of struct fields");
