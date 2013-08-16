#include "PrintFunctionNames.hpp"

#include <llvm/Analysis/DebugInfo.h>
#include <llvm/Instruction.h>
#include <llvm/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ValueSymbolTable.h>

#include <iostream>
#include <sstream>

using namespace std;
using namespace llvm;


bool PrintFunctionNames::runOnModule(Module &M) {

  errs() << "Pass PrintFunctionNames\n";

  // iterating through functions
  for(Module::iterator f = M.begin(), fe = M.end(); f != fe; f++) {

    if (!f->isDeclaration()) {
      // finding and printing file information 
      BasicBlock &block = f->getEntryBlock();
      Instruction &inst = block.front();
      
      if (MDNode *node = inst.getMetadata("dbg")) {
	DILocation loc(node);
	errs() << "File name: " << loc.getFilename() << "\t";
      }
    }
      
    // finding and printing function informatonion
    errs() << "Function name: " << f->getName() << "\n";
  }

  return false;
}  


void PrintFunctionNames::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
}


char PrintFunctionNames::ID = 0;
static const RegisterPass<PrintFunctionNames> registration("print-names", "Printing function names");
