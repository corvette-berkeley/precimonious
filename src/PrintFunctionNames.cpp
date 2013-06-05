#include "PrintFunctionNames.hpp"

#include <llvm/Module.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>

#include <iostream>
#include <sstream>

using namespace std;
using namespace llvm;


cl::opt<string> OutputFileName("output", cl::value_desc("filename"), cl::desc("Output XML file"));

bool PrintFunctionNames::runOnModule(Module &M) {

  // iterating through functions
  for(Module::iterator f = M.begin(), fe = M.end(); f != fe; f++) {
    errs() << "Function name: " << f->getName() << "\n";
  }

  return false;
}  


void PrintFunctionNames::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
}


char PrintFunctionNames::ID = 0;
static const RegisterPass<PrintFunctionNames> registration("print-names", "Printing function names");
