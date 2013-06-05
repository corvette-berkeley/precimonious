#include "RemoveDeadCasting.hpp"

#include <llvm/Module.h>
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <llvm/Support/raw_ostream.h>

#include <list>
#include <fstream>

void RemoveDeadCasting::doInitialization(Module &M) {
  debugInfo.processModule(M);

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


bool RemoveDeadCasting::runOnModule(Module &M) {
  doInitialization(M);

  errs() << "Running RemoveDeadCastings\n";
  for (Module::iterator f = M.begin(), fe = M.end(); f != fe; f++) {
    if (!f->isDeclaration()) {
      runOnFunction(*f);
    }
  }
  errs() << "Done!\n";

  return false;
}

// optimize the following pattern
// inst0: old_type r1
// inst1: r2 = (new_type) r1
// inst2: r3 = (old_type) r2
// if r2 is only used for casting
// then remove inst1 and inst2, and
// replace all uses of r3 with r1
bool RemoveDeadCasting::runOnFunction(Function &F) {
  string name = F.getName();

  if (! F.isDeclaration()  && (includedFunctions.find(name) != includedFunctions.end()) && (excludedFunctions.find(name) == excludedFunctions.end())) {
    errs() << "=== Exploring function: " << F.getName() << "\n";
    list<Instruction*> dead_cast;
    for (Function::iterator b = F.begin(), be = F.end(); b != be; b++) {
      for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {
        Instruction *inst = i;
        if (CastInst* r2 = dyn_cast<CastInst>(inst)) {
          Value *r1 = r2->getOperand(0);
          Type* old_type = r1->getType();

          if (old_type->getTypeID() == Type::FloatTyID ||
              old_type->getTypeID() == Type::DoubleTyID ||
              old_type->getTypeID() == Type::X86_FP80TyID) 
          {
            bool use_only_for_cast = true;
            for (Value::use_iterator use = r2->use_begin(); use != r2->use_end(); use++) {
              if (CastInst* r3 = dyn_cast<CastInst>(*use)) {
                Type* new_type = r3->getType();
                if (new_type->getTypeID() == old_type->getTypeID()) {
                  // replace uses of r3 with r1
                  r3->replaceAllUsesWith(r1);
                  // remove r3 = (old_type) r2
                  r3->eraseFromParent();
                } else {
                  use_only_for_cast = false;
                }
              } else {
                use_only_for_cast = false;
              }
            }

            if (use_only_for_cast) {
              dead_cast.push_back(r2);
            }
          }
        }
      }
    }

    for (list<Instruction*>::iterator i = dead_cast.begin(); i != dead_cast.end(); i++) {
      Instruction *inst = *i;
      inst->eraseFromParent();
    }
  }
  return false;
}


void RemoveDeadCasting::getAnalysisUsage(AnalysisUsage &AU __attribute__((unused))) const {
  // no dependencies
  return;
}

char RemoveDeadCasting::ID = 0;
static const RegisterPass<RemoveDeadCasting> registration("remove-dead-casting", "Remove casting that is not used");
