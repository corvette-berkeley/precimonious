//#include "CollectVars.hpp"
#include "ChangePrecision.hpp"
#include "Transformer.hpp"

#include <llvm/Constants.h>
#include <llvm/IntrinsicInst.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Module.h>
#include <llvm/ValueSymbolTable.h>
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>

#include <iostream>
#include <fstream>

cl::opt<string> ConfigFileName("config", cl::value_desc("filename"), cl::desc("Configuration input file"), cl::init("config.txt"));

bool ChangePrecision::doInitialization(Module &module) {
  
  debugInfo.processModule(module);
  
  //CollectVars &vars = getAnalysis<CollectVars>();
  //locals = vars.getLocals();
  
  // read variables from config file
  
  string sfunction;
  string svariable;
  
  ifstream config(ConfigFileName.c_str());
  if (config.is_open()) {
    
    while(config >> sfunction >> svariable) {
      const ValueSymbolTable& moduleSymbolTable = module.getValueSymbolTable();
      Value *vfunction = moduleSymbolTable.lookup(sfunction);
      Function *function = dyn_cast<Function>(vfunction);
      
      if (function) {
	const ValueSymbolTable& symbolTable = function->getValueSymbolTable();
	Value* target = symbolTable.lookup(svariable);
	
	if (target) {
	  locals[function].push_back(target);
	}
      }

    } // while
  }
  else {
    errs() << "ERROR: Could not open configuration file\n";
  }

  return false;
}


// NOTE: bitcode is verified at the end.

bool ChangePrecision::runOnModule(Module &M) {
  
  doInitialization(M);
  
  for(Module::iterator function = M.begin(), end = M.end(); function != end; function++) {
    
    map<const Function*, Variables>::iterator it = locals.find(function);
    
    if (it != locals.end()) {      
      Variables &vars = locals[function];
      
      for(unsigned int i = 0; i < vars.size(); i++) {	  
	
	// creating new AllocaInst
	Value* target = vars[i];
	//errs() << "Changing variable: " << function->getName() << ":" << target->getName() << "\n";
	
	AllocaInst* old_target = dyn_cast<AllocaInst>(target);	  
	LLVMContext &context = function->getContext();
	Type* type = Type::getFloatTy(context);
	unsigned alignment = 4;
	
	AllocaInst* new_target = new AllocaInst(type, 0, alignment, "new", old_target);
	new_target->takeName(old_target);
	
	// iterating through instructions using old AllocaInst
	Value::use_iterator it = target->use_begin();
	for(; it != target->use_end(); it++) {
	  Transformer::transform(it, new_target, type);
	}	  
	old_target->eraseFromParent();	  
      }
    }	    
    //function->dump();
  }  
  
  return false;
}


void ChangePrecision::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  //AU.addRequired<CollectVars>();
}


char ChangePrecision::ID = 0;
static const RegisterPass<ChangePrecision> registration("d2f", "Reduce the precision of a program");
