#include "CreateIDBitcode.hpp"
#include "Variables.hpp"
#include "Transformer.hpp"
#include "ParseConfig.hpp"

#include <llvm/Analysis/DebugInfo.h>
#include <llvm/Analysis/DIBuilder.h>
#include <llvm/Support/CommandLine.h>
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


cl::opt<string> OutputFileName("output", cl::value_desc("filename"), cl::desc("File name for transformed bitcode (used in regression tests)"));


ConstantInt* Variables::getInt32(int n) {
  static llvm::LLVMContext& global = llvm::getGlobalContext();
  return llvm::ConstantInt::get(llvm::Type::getInt32Ty(global), n);
}


ConstantInt* Variables::getInt64(int n) {
  static llvm::LLVMContext& global = llvm::getGlobalContext();
  return llvm::ConstantInt::get(llvm::Type::getInt64Ty(global), n);
}

static bool diffTypes(Type *type1, Type *type2) {

#ifdef DEBUG
  errs() << "COMPARING " << type1 << " AND " << type2;
#endif
  
   unsigned int typeID1 = type1->getTypeID();
   unsigned int typeID2 = type2->getTypeID();

   if (typeID1 != typeID2) {
     return true;
   }
   else {

     // Case: pointer to FP (arrays passed as parameter)
     if (PointerType *ptype1 = dyn_cast<PointerType>(type1)) {
       if (PointerType *ptype2 = dyn_cast<PointerType>(type2)) {

         Type *elementType1 = ptype1->getElementType();
         Type *elementType2 = ptype2->getElementType();

         if (elementType1->isFloatingPointTy() && elementType2->isFloatingPointTy()) {
           if (elementType1->getTypeID() != elementType2->getTypeID()) {
             return true;
           }
         }
	 else if (PointerType *ep1 = dyn_cast<PointerType>(elementType1)) {
	   if (PointerType *ep2 = dyn_cast<PointerType>(elementType2)) {
	     return diffTypes(ep1, ep2);
	   }
	 }
       }
     }

     // Case: Arrays FP
     if (ArrayType *ptype1 = dyn_cast<ArrayType>(type1)) {
       if (ArrayType *ptype2 = dyn_cast<ArrayType>(type2)) {

         Type *elementType1 = ptype1->getElementType();
         Type *elementType2 = ptype2->getElementType();

         if (elementType1->isFloatingPointTy() && elementType2->isFloatingPointTy()) {
           if (elementType1->getTypeID() != elementType2->getTypeID()) {
             return true;
           }
         }
         else if (ArrayType *ep1 = dyn_cast<ArrayType>(elementType1)) {
           if (ArrayType *ep2 = dyn_cast<ArrayType>(elementType2)) {
             return diffTypes(ep1, ep2);
           }
         }
       }
     }

   }
   return false;
}

void Variables::doInitialization(Module &module) {
  
  debugInfo.processModule(module);
  ParseConfig &parseConfig = getAnalysis<ParseConfig>();
  changes = parseConfig.getChanges();

  return;
}


unsigned Variables::getAlignment(Type* type) {
  unsigned alignment;    
  switch(type->getTypeID()) {
  case Type::FloatTyID:
    alignment = 4;
    break;
  case Type::DoubleTyID:
    alignment = 8;
    break;
  case Type::X86_FP80TyID:
    alignment = 16;
    break;
  default:
    alignment = 0;
  }
  return alignment;
}


void Variables::changeGlobal(Change* change, Module &module) {

  GlobalValue* oldTarget = dyn_cast<GlobalValue>(change->getValue());
  Type* oldType = oldTarget->getType()->getElementType();
  Type* newType = change->getType()[0];
  errs() << "Changing the precision of variable \"" << oldTarget->getName() << "\" from " << *oldType << " to " << *newType << ".\n";

  if (diffTypes(oldType, newType)) {      
    Constant *initializer;
    GlobalVariable* newTarget;

    if (PointerType *newPointerType = dyn_cast<PointerType>(newType)) {
      initializer = ConstantPointerNull::get(newPointerType);
      newTarget = new GlobalVariable(module, newType, false, GlobalValue::CommonLinkage, initializer, "");
    }
    else if (ArrayType * atype = dyn_cast<ArrayType>(newType)) {

      // preparing initializer
      Type *temp = Type::getFloatTy(module.getContext());
      vector<Constant*> operands;
      operands.push_back(ConstantFP::get(temp, 0));
      ArrayRef<Constant*> *arrayRef = new ArrayRef<Constant*>(operands);
      initializer = ConstantArray::get(atype, *arrayRef);

      newTarget = new GlobalVariable(module, newType, false, GlobalValue::CommonLinkage, initializer, "");
    }
    else {
      initializer = ConstantFP::get(newType, 0);
      newTarget = new GlobalVariable(module, newType, false, GlobalValue::CommonLinkage, initializer, "");
    }

    /*
    GlobalVariable* newTarget = new GlobalVariable(module, newType, false, GlobalValue::CommonLinkage, initializer, "");
    */

    unsigned alignment = getAlignment(newType);
    newTarget->setAlignment(alignment);

    newTarget->takeName(oldTarget);
    
    // iterating through instructions using old AllocaInst
    Value::use_iterator it = oldTarget->use_begin();
    for(; it != oldTarget->use_end(); it++) {
      Transformer::transform(it, newTarget, oldTarget, newType, oldType, alignment);
    }	  
    //oldTarget->eraseFromParent();
  }
  else {
    errs() << "No changes required.\n";
  }
  return;
}


static bool diffStructTypes(StructType *type1, StructType *type2) {
  int num = type1->getNumElements();

  // assum same number of elements
  for(int i = 0; i < num; i++) {
    if (diffTypes(type1->getElementType(i), type2->getElementType(i))) {
      return true;
    }
  }
  return false;
}


AllocaInst* Variables::changeLocal(Value* value, Type* newType) {

  AllocaInst* newTarget = NULL;
  vector<Instruction*> erase;

  if (AllocaInst *oldTarget = dyn_cast<AllocaInst>(value)) {
    Type* oldType = oldTarget->getType()->getElementType();

    errs() << "Changing the precision of variable \"" << oldTarget->getName() << "\" from " << *oldType 
	   << " to " << *newType << ".\n";

    if (diffTypes(oldType, newType)) {      
      unsigned alignment = getAlignment(newType);

      newTarget = new AllocaInst(newType, 0, alignment, "new", oldTarget);
      newTarget->takeName(oldTarget);

      // iterating through instructions using old AllocaInst
      Value::use_iterator it = oldTarget->use_begin();
      for(; it != oldTarget->use_end(); it++) {
        bool is_erased = Transformer::transform(it, newTarget, oldTarget, newType, oldType, alignment);

	if (!is_erased) {
	  erase.push_back(dyn_cast<Instruction>(*it));
	}
      }
      // erasing old uses of instructions
      for(unsigned int i = 0; i < erase.size(); i++) {
	erase[i]->eraseFromParent();
      }
      // erase old instruction
      //oldTarget->eraseFromParent();      
    }
    else {
      errs() << "\tNo changes required.\n";
    }
  }
  else if (Argument *argument = dyn_cast<Argument>(value)){
    errs() << "WARNING: Function argument instead of Alloca for: " << argument->getName() << ".\n";
  }

  return newTarget;
}


AllocaInst* Variables::changeLocal(Change* change) {

  Type* newType = change->getType()[0];
  if (ArrayType* arrayType = dyn_cast<ArrayType>(newType)) {
    return changeLocal(change->getValue(), arrayType);
  } else if (PointerType* pointerType = dyn_cast<PointerType>(newType)) {
    return changeLocal(change->getValue(), pointerType);
  } else if (StructType* structType = dyn_cast<StructType>(newType)) {
    return changeLocal(change->getValue(), structType/*, change->getField()*/);
  } else {
    return changeLocal(change->getValue(), newType);
  }
}


AllocaInst* Variables::changeLocal(Value* value, PointerType* newType) {
  AllocaInst *oldTarget = dyn_cast<AllocaInst>(value);
  PointerType* oldPointerType = dyn_cast<PointerType>(oldTarget->getType());
  PointerType *oldType = dyn_cast<PointerType>(oldPointerType->getElementType());
  AllocaInst *newTarget = NULL;

  errs() << "Changing the precision of pointer variable \"" << oldTarget->getName() << "\" from " << *oldType 
	 << " to " << *newType << ".\n";

  if (diffTypes(newType, oldType)) {
    newTarget = new AllocaInst(newType, getInt32(1), "", oldTarget);

    // we are not calling getAlignment because in this case double requires 16. Investigate further.
    unsigned alignment;
    switch(newType->getElementType()->getTypeID()) {
      case Type::FloatTyID: 
        alignment = 4;
        break;
      case Type::DoubleTyID:
        alignment = 8;
        break;
      case Type::X86_FP80TyID:
        alignment = 16;
      break;
    default:
      alignment = 0;
    } 
    
    newTarget->setAlignment(alignment); // depends on type? 8 for float? 16 for double?
    newTarget->takeName(oldTarget);

    // iterating through instructions using old AllocaInst
    vector<Instruction*> erase;
    Value::use_iterator it = oldTarget->use_begin();

#ifdef DEBUG
    errs() << "\nOld target: ";
    oldTarget->dump();
#endif

    for(; it != oldTarget->use_end(); it++) {
#ifdef DEBUG
      errs() << "\nA use: ";
      it->dump();

      errs() << "\n===============================\n";
      errs() << "\nTransforming use\n";
#endif

      bool is_erased = Transformer::transform(it, newTarget, oldTarget, newType, oldType, alignment);

      if (!is_erased) {
        erase.push_back(dyn_cast<Instruction>(*it));
      }

#ifdef DEBUG
      errs() << "\nDone transforming use\n";
#endif
    }
    
    // erasing uses of old instructions
    for(unsigned int i = 0; i < erase.size(); i++) {
      erase[i]->eraseFromParent();
    }
    // erase old instruction
    //oldTarget->eraseFromParent();

#ifdef DEBUG
    errs() << "DONE ALL TRANSFORMATION FOR POINTER\n";
#endif

  } else {
    errs() << "\tNo changes required.\n";
  }

  return newTarget;
}


AllocaInst* Variables::changeLocal(Value* value, ArrayType* newType) {

  AllocaInst* oldTarget = dyn_cast<AllocaInst>(value);
  PointerType* oldPointerType = dyn_cast<PointerType>(oldTarget->getType());
  ArrayType* oldType = dyn_cast<ArrayType>(oldPointerType->getElementType());
  AllocaInst* newTarget = NULL;

  errs() << "Changing the precision of variable \"" << oldTarget->getName() << "\" from " << *oldType 
	 << " to " << *newType << ".\n";

  if (newType->getElementType()->getTypeID() != oldType->getElementType()->getTypeID()) {

    newTarget = new AllocaInst(newType, getInt32(1), "", oldTarget);
    
    // we are not calling getAlignment because in this case double requires 16. Investigate further.
    unsigned alignment;
    switch(newType->getElementType()->getTypeID()) {
    case Type::FloatTyID: 
      alignment = 4;
      break;
    case Type::DoubleTyID:
      alignment = 16;
      break;
    case Type::X86_FP80TyID:
      alignment = 16;
      break;
    default:
      alignment = 0;
    } 
    
    newTarget->setAlignment(alignment); // depends on type? 8 for float? 16 for double?
    newTarget->takeName(oldTarget);
    
    // iterating through instructions using old AllocaInst
    vector<Instruction*> erase;
    Value::use_iterator it = oldTarget->use_begin();
    for(; it != oldTarget->use_end(); it++) {
      bool is_erased = Transformer::transform(it, newTarget, oldTarget, newType, oldType, alignment);

      if (!is_erased)
        erase.push_back(dyn_cast<Instruction>(*it));
    }	  
    
    // erasing uses of old instructions
    for(unsigned int i = 0; i < erase.size(); i++) {
      erase[i]->eraseFromParent();
    }
    // erase old instruction
    //oldTarget->eraseFromParent();
  }
  else {
    errs() << "\tNo changes required.\n";    
  }

  return newTarget;
}


AllocaInst* Variables::changeLocal(Value* value, StructType* newType/*, int field*/) {

  errs() << "At changeLocalStruct\n";
  AllocaInst* newTarget = NULL;
  vector<Instruction*> erase;

  if (AllocaInst *oldTarget = dyn_cast<AllocaInst>(value)) {
    if (PointerType *oldPointer = dyn_cast<PointerType>(oldTarget->getType())) {
      if (StructType *oldType = dyn_cast<StructType>(oldPointer->getElementType())) {
	
	errs() << "Changing the precision of variable \"" << oldTarget->getName() << "\" from " << *oldType 
	       << " to " << *newType << ".\n";
      
	if (diffStructTypes(oldType, newType)) {
	  unsigned alignment = oldTarget->getAlignment();
	  
	  newTarget = new AllocaInst(newType, 0, alignment, "new", oldTarget);
	  newTarget->takeName(oldTarget);
	
	  // iterating through instructions using old getelementptr instructions
	  Value::use_iterator it = oldTarget->use_begin();
	  for(; it != oldTarget->use_end(); it++) {
	    
	    if (GetElementPtrInst *getElementPtrInst = dyn_cast<GetElementPtrInst>(*it)) {
	      if (ConstantInt *constantIntIndex = dyn_cast<ConstantInt>(getElementPtrInst->getOperand(2))) {
		unsigned int index = constantIntIndex->getLimitedValue(); // the index of the field accessed by this use
		
		Type *newFieldType = newType->getElementType(index);
		Type *oldFieldType = oldType->getElementType(index);
		unsigned alignment = getAlignment(newFieldType); // 4 hard coded for now
		bool is_erased = Transformer::transform(it, newTarget, oldTarget, newFieldType, oldFieldType, alignment);
		
		if (!is_erased) {
		  erase.push_back(dyn_cast<Instruction>(*it));
		}  
	      }
	    }
	    else {
	      errs() << "WARNING: unexpected use of struct\n";
	    }
	  }
	  
	  // erasing uses of old instructions
	  for(unsigned int i = 0; i < erase.size(); i++) {
	    erase[i]->eraseFromParent();
	  }
	  
	}
	else {
	  errs() << "\tNo changes required.\n";
	}
      } 
    }
  }
  return newTarget;
}


ConstantInt* Variables::getSizeInBits(Type *type) {
  ConstantInt *size = NULL;
  ArrayType *arrayType = dyn_cast<ArrayType>(type);
  switch (arrayType->getElementType()->getTypeID()) {
  case Type::FloatTyID: {
    int isize = arrayType->getNumElements() * 32;
    size = getInt64(isize);
    break;
  }
  case Type::DoubleTyID: {
    int isize = arrayType->getNumElements() * 64;
    size = getInt64(isize);
    break;
  }
  default:
    errs() << "WARNING: Unhandled type @ getSizeInBits\n";
    exit(1);
  }
  return size;
}


ConstantInt* Variables::getAlignmentInBits(Type *type) {
  ConstantInt *alignment = NULL;
  ArrayType *arrayType = dyn_cast<ArrayType>(type);
  switch (arrayType->getElementType()->getTypeID()) {
  case Type::FloatTyID:
    alignment = getInt64(32);
    break;
  case Type::DoubleTyID:
    alignment = getInt64(64);
    break;
  default:
    errs() << "WARNING: Unhandled type @ getAlignmentInBits\n";
    exit(1);
  }
  return alignment;
}


MDNode* Variables::getTypeMetadata(Module &module, DIVariable &oldDIVar, Type *newType) {

  // constructing new type descriptor
  const DIType &oldDIType = oldDIVar.getType();  
  vector<Value*> operands;

  switch(newType->getTypeID()) {
  case Type::FloatTyID:
    operands.push_back(getInt32(524324));
    operands.push_back(oldDIType->getOperand(1)); // preserve old context
    operands.push_back(MDString::get(module.getContext(), "float"));
    operands.push_back(oldDIType->getOperand(3)); // preserve old file descriptor
    operands.push_back(getInt32(0));
    operands.push_back(getInt64(32));
    operands.push_back(getInt64(32));
    operands.push_back(getInt64(0));
    operands.push_back(getInt32(0));
    operands.push_back(getInt32(4));
    break;
  case Type::DoubleTyID:   
    operands.push_back(getInt32(524324));
    operands.push_back(oldDIType->getOperand(1)); // preserve old context
    operands.push_back(MDString::get(module.getContext(), "double"));
    operands.push_back(oldDIType->getOperand(3)); // preserve old file descriptor
    operands.push_back(getInt32(0));
    operands.push_back(getInt64(64));
    operands.push_back(getInt64(64));
    operands.push_back(getInt64(0));
    operands.push_back(getInt32(0));
    operands.push_back(getInt32(4));
    break;
  case Type::ArrayTyID: {
    Type *elementType = (dyn_cast<ArrayType>(newType))->getElementType();
    operands.push_back(getInt32(720897));
    operands.push_back(oldDIType->getOperand(1)); // preserve old context
    operands.push_back(MDString::get(module.getContext(), ""));
    operands.push_back(oldDIType->getOperand(3)); // preserve old file descriptor
    operands.push_back(getInt32(0));
    operands.push_back(getSizeInBits(newType)); // size in bits = size * array elements
    operands.push_back(getAlignmentInBits(newType)); // alignment in bits
    operands.push_back(getInt32(0));
    operands.push_back(getInt32(0));
    operands.push_back(getTypeMetadata(module, oldDIVar, elementType)); // element basic type
    operands.push_back(oldDIType->getOperand(10)); // subrange should be the same
    operands.push_back(getInt32(0));
    operands.push_back(getInt32(0));
    break; 
  }
  case Type::PointerTyID: { // added on 07/26
    Type *elementType = (dyn_cast<PointerType>(newType))->getElementType();
    operands.push_back(getInt32(786447));
    operands.push_back(oldDIType->getOperand(1)); // preserve old file descriptor
    operands.push_back(oldDIType->getOperand(2)); // preserve old context
    operands.push_back(oldDIType->getOperand(3)); // preserve old name
    operands.push_back(oldDIType->getOperand(4)); // preserve old line number
    operands.push_back(oldDIType->getOperand(5)); // preserve old size in bits
    operands.push_back(oldDIType->getOperand(6)); // preserve old align in bits
    operands.push_back(oldDIType->getOperand(7)); // preserve old offset in bits
    operands.push_back(oldDIType->getOperand(8)); // preserve old flags
    operands.push_back(getTypeMetadata(module, oldDIVar, elementType)); // element basic type
    break; 
  }
  default:
    errs() << "WARNING: getTypeMetadata.\n";
    newType->dump();
    errs() << "\n";
    exit(1);
  }
  
  ArrayRef<Value*> *arrayRefOperands = new ArrayRef<Value*>(operands);
  return MDNode::get(module.getContext(), *arrayRefOperands);
}


void Variables::updateMetadata(Module& module, Value* oldTarget, Value* newTarget, Type* newType) {

  vector<Instruction*> to_remove;
  if (newTarget) {
    errs() << "\tChanging metadata for: " << newTarget->getName() << "\n";
    bool changed = false;

    for(Module::iterator f = module.begin(), fe = module.end(); f != fe; f++) {
      for(Function::iterator b = f->begin(), be = f->end(); b != be; b++) {
	for(BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {
	  
	  if (DbgDeclareInst *oldDeclare = dyn_cast<DbgDeclareInst>(i)) {
	    if (Value *address = oldDeclare->getAddress()) {
	      if (AllocaInst *allocaInst = dyn_cast<AllocaInst>(address)) {
		if (allocaInst == oldTarget) { // the alloca we are looking for

		  DIVariable oldDIVar(oldDeclare->getVariable());
		  MDNode* newDIType = getTypeMetadata(module, oldDIVar, newType);
		  
		  // construct new DIVariable with new type descriptor
		  vector<Value*> doperands;
		  for(unsigned i = 0; i < oldDIVar->getNumOperands(); i++) {
		    if (i == 5) { // the argument that corresponds to the type descriptor
		      doperands.push_back(newDIType);
		    }
		    else {
		      doperands.push_back(oldDIVar->getOperand(i)); // preserve other descriptors
		    }
		  }
		  ArrayRef<Value*> *arrayRefDOperands = new ArrayRef<Value*>(doperands);
		  MDNode* newMDNode = MDNode::get(module.getContext(), *arrayRefDOperands);
		  DIVariable newDIVar(newMDNode);
		  
		  // insert new declare instruction
		  DIBuilder* builder = new DIBuilder(module);
		  Instruction *newDeclare = builder->insertDeclare(newTarget, newDIVar, oldDeclare);
		  
		  // make sure the declare instruction preserves its own metadata
		  unsigned id = 0;
		  if (oldDeclare->getMetadata(id)) {
		    newDeclare->setMetadata(id, oldDeclare->getMetadata(id));
		  }
		  to_remove.push_back(oldDeclare); // can't erase while iterating through instructions
		  changed = true;
		}
	      }
	    }
	  }
	}
      }
    }
    for(unsigned i = 0; i < to_remove.size(); i++) {
      to_remove[i]->eraseFromParent();
    }
    if (!changed) {
      errs() << "\tNo metadata to change\n";
    }
  }
  return;
}


bool Variables::runOnModule(Module &module) {
  errs() << "Running Variables\n";
  doInitialization(module);

  vector<Change*>::iterator it;

  for(it = changes[GLOBALVAR].begin(); it != changes[GLOBALVAR].end(); it++) {
    changeGlobal(*it, module); // TODO: return value and metadata
  }

  for(it = changes[LOCALVAR].begin(); it != changes[LOCALVAR].end(); it++) {
    AllocaInst* newTarget = changeLocal(*it);
    if (newTarget) {
      errs() << "\tProcessed local variable: " << newTarget->getName() << "\n";
      updateMetadata(module, (*it)->getValue(), newTarget, (*it)->getType()[0]);

#ifdef DEBUG
      verifyModule(module, AbortProcessAction);
      errs() << "**** MODULE VERIFIES after a single change ****\n";
#endif
    }
  }

  return true;
}


void Variables::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<ParseConfig>();
}


char Variables::ID = 0;
static const RegisterPass<Variables> registration("variables", "Change the precision of variables");
