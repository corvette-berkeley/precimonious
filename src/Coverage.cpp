#include "Coverage.hpp"

#include <llvm/Module.h>
#include <llvm/Constants.h>
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>

#include <fstream>

void Coverage::doInitialization(Module &module) 
{
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

bool Coverage::runOnModule(Module &module)
{
  doInitialization(module);

  // iterate through functions in module
  for (Module::iterator f = module.begin(), fe = module.end(); f != fe; f++)
  {
    runOnFunction(module, *f);
  }

  return true;
}

bool Coverage::runOnFunction(Module &module, Function &function)
{
  int blockId = 0;
  int branchId = 0;
  if (! function.isDeclaration() && (excludedFunctions.find(function.getName()) == excludedFunctions.end()))
  {
    bool printBranchTo = false;
    for (Function::iterator b = function.begin(), be = function.end(); b != be; b++)
    {
      for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++)
      {
        blockId++;

        //
        // printing target block id
        //
        if (!dyn_cast<PHINode>(i) && printBranchTo)
        {
          std::vector<Value*> args;

          IntegerType *int32Type = Type::getInt32Ty(module.getContext());

          ConstantInt* arg0 = ConstantInt::getSigned(int32Type, blockId);

          args.push_back((Value*)arg0);

          ArrayRef<Value*> *arrayRef = new ArrayRef<Value*>(args);

          CallInst::Create(module.getFunction("printBranchTo"), *arrayRef, "", i);
          printBranchTo = false;
        }

        //
        // printing branch id
        // TODO: switch instruction
        //
        if (dyn_cast<BranchInst>(i) || dyn_cast<IndirectBrInst>(i)) 
        {
          branchId++;
          std::vector<Value*> args;

          IntegerType *int32Type = Type::getInt32Ty(module.getContext());

          ConstantInt* arg0 = ConstantInt::getSigned(int32Type, blockId);
          ConstantInt* arg1 = ConstantInt::getSigned(int32Type, branchId);

          args.push_back((Value*)arg0);
          args.push_back((Value*)arg1);

          ArrayRef<Value*> *arrayRef = new ArrayRef<Value*>(args);

          CallInst::Create(module.getFunction("printBranch"), *arrayRef, "", i);
          printBranchTo = true;
        }
      }
    }
  }

  return true;
}

void Coverage::getAnalysisUsage(AnalysisUsage &AU __attribute__((unused))) const 
{
  return;
}

char Coverage::ID = 0;

static const RegisterPass<Coverage> registration("coverage", "Prints Coverage Trace");
