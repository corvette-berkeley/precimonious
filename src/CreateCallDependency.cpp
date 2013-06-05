#include "CreateCallDependency.hpp"

#include <llvm/Module.h>
#include <llvm/Instructions.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/CommandLine.h>
#include <set>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;
using namespace llvm;

/**
 * Declaring call-main as an option to pass name of
 * main function to this pass.
 */
cl::opt<string> CallMainName("call-main", cl::value_desc("name"), cl::desc("Name of main function"), cl::init("main"));

bool CreateCallDependency::doInitialization(Module &M) {
	debugInfo.processModule(M);

	return true;
}

bool CreateCallDependency::runOnModule(Module &M) {
	doInitialization(M);

	for (Module::iterator f = M.begin(), fe = M.end(); f != fe; f++) {
		if (!f->isDeclaration()) {
			nameToCall[f->getName()] = f;
		}
	}

	findDependency(CallMainName);

  calls.insert(CallMainName);

  findUsedGlobalVar(M);

	doFinalization();

	return true;
}

set<string> CreateCallDependency::findCalledFunctions(string call) {
	set<string> calledFunctions;
	Function *F = nameToCall[call];

	for (Function::iterator b = F->begin(), be = F->end(); b != be; b++) {
		for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {
			if (CallInst *call = dyn_cast<CallInst>(i)) {
				if (Function* f = call->getCalledFunction()) {
					if (!f->isDeclaration()) {
						string calledFunction = f->getName();
						calledFunctions.insert(calledFunction);
					}
				}
			}
		} 
	}

	return calledFunctions;
}

void CreateCallDependency::findDependency(string call) {
	set<string> calledFunctions = findCalledFunctions(call);
	set<string>::iterator cfIt;

	for (cfIt = calledFunctions.begin(); cfIt != calledFunctions.end(); ++cfIt) {
		string calledFunction = *cfIt;
		if (calls.find(calledFunction) == calls.end()) {
			calls.insert(calledFunction);
			findDependency(calledFunction);
		}
	}
}

void CreateCallDependency::findUsedGlobalVar(Module &M) {
  // 
  // iterate through all global variables
  //
  for (Module::global_iterator it = M.global_begin(); it != M.global_end(); it++) {
    Value *value = it;
    //
    // iterate through all global variable usage
    // if global variable is used in one of the functions
    // in calls set, save global variable
    // into globalVars set.
    //
    if (GlobalVariable *global = dyn_cast<GlobalVariable>(value)) {
      for (Value::use_iterator uit = global->use_begin(); uit != global->use_end(); uit++) {
        if (Instruction *inst = dyn_cast<Instruction>(*uit)) {
          Function *f = inst->getParent()->getParent();
          string functionName = f->getName();
          if (calls.find(functionName) != calls.end()) {
            globalVars.insert(global->getName());
          }
        }
      }
    }
  }
}

void CreateCallDependency::getAnalysisUsage(AnalysisUsage &AU) const {
	AU.setPreservesAll();
}

void CreateCallDependency::doFinalization() {
	set<string>::iterator i;

  ofstream includeFile ("include.txt");
  if (includeFile.is_open()) {
    for (i = calls.begin(); i != calls.end(); ++i) {
      includeFile << *i << "\n";
    }
    includeFile.close();
  }

  ofstream globalIncludeFile ("include_global.txt");
  if (globalIncludeFile.is_open()) {
    for (i = globalVars.begin(); i != globalVars.end(); ++i) {
      globalIncludeFile << *i << "\n";
    }
    globalIncludeFile.close();
  }

}

char CreateCallDependency::ID = 0;

static const RegisterPass<CreateCallDependency> registration("create-call-dependency", "Create call depepdency");
