#include "Transformer.hpp"
#include <llvm/Constants.h>
#include <llvm/IntrinsicInst.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Module.h>
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <llvm/Support/raw_ostream.h>
#include <llvm/LLVMContext.h>

#include <stdio.h>

#define UNUSED __attribute__((__unused__))

using namespace llvm;


ConstantInt* Transformer::getInt32(int n) {
  static llvm::LLVMContext& global = llvm::getGlobalContext();
  return llvm::ConstantInt::get(llvm::Type::getInt32Ty(global), n);
}


ConstantInt* Transformer::getInt64(int n) {
  static llvm::LLVMContext& global = llvm::getGlobalContext();
  return llvm::ConstantInt::get(llvm::Type::getInt64Ty(global), n);
}


Type* getElementType(Type* type) {
  Type *elementType = NULL;
  if (ArrayType *arrayType = dyn_cast<ArrayType>(type)) {
    elementType = arrayType->getElementType();
  } else if (PointerType *pointerType = dyn_cast<PointerType>(type)) {
    elementType = pointerType->getElementType();
  }
  return elementType;
}


bool Transformer::transform(value_use_iterator<User> it, Value* newTarget, Value* oldTarget, Type* newType, Type* oldType, unsigned alignment) {
  if (LoadInst *inst = dyn_cast<LoadInst>(*it)) {
    return transform(*inst, newTarget, oldTarget, newType, oldType, alignment);
  } else if(StoreInst *inst = dyn_cast<StoreInst>(*it)) {
    return transform(*inst, newTarget, oldTarget, newType, oldType, alignment);
  } else if(BitCastInst *inst = dyn_cast<BitCastInst>(*it)) {
    return transform(*inst, newTarget, oldTarget, newType, oldType, alignment);
  } else if(GetElementPtrInst *inst = dyn_cast<GetElementPtrInst>(*it)) {
    return transform(*inst, newTarget, oldTarget, newType, oldType, alignment);
  } else if(CallInst *inst = dyn_cast<CallInst>(*it)) {
    return transform(*inst, newTarget, oldTarget, newType, oldType, alignment);
  }
  return false;
} 

bool Transformer::transform(LoadInst& inst, Value* newTarget, Value* oldTarget UNUSED,  Type* newType, Type* oldType, unsigned alignment) {
#ifdef DEBUG
  errs() << "Transforming LOADINST" << &oldTarget << "\n";
#endif

  LoadInst* loadInst = &inst;
  LoadInst* newLoad = new LoadInst(newTarget, "", false, alignment, loadInst);

#ifdef DEBUG
  errs() << "\n=Transforming LOADINST";
  errs() << "\nOld: ";
  loadInst->dump();
  errs() << "\nNew: ";
  newLoad->dump();
#endif

  if (dyn_cast<PointerType>(newType)) {
    vector<Instruction*> erase;
    Value::use_iterator it = loadInst->use_begin();
    for(; it != loadInst->use_end(); it++) {
      if (CallInst* callInst = dyn_cast<CallInst>(*it)) {
        BitCastInst *bitCast = new BitCastInst(newLoad, oldType, "", callInst);
        for (unsigned i = 0; i < callInst->getNumOperands(); i++) {
          if (callInst->getOperand(i) == loadInst)
            callInst->setArgOperand(i, bitCast);
        }
      } else {
        bool is_erased = Transformer::transform(it, newLoad, loadInst, newType, oldType, alignment);
        if (!is_erased)
          erase.push_back(dyn_cast<Instruction>(*it));
      }
    }

    // erasing old instructions
    for (unsigned int i = 0; i < erase.size(); i++) {
      erase[i]->eraseFromParent();
    }
  } else {
    if (newType->getTypeID() > oldType->getTypeID()) {
      FPTruncInst *trunc = new FPTruncInst(newLoad, oldType, "", loadInst);
      loadInst->replaceAllUsesWith(trunc);
    }
    else {
      FPExtInst *ext = new FPExtInst(newLoad, oldType, "", loadInst);
      loadInst->replaceAllUsesWith(ext);
    }
  } 

  // the old target was not erased
  return false;
}


bool Transformer::transform(StoreInst& inst, Value* newTarget, Value* oldTarget, Type* newType, Type* oldType, unsigned alignment) {

#ifdef DEBUG
  errs() << "\nTransforming STOREINST";
  errs() << "\nOld: ";
  errs() << "\tTransforming from " << *oldType << " to " << *newType << "\n"; 
  inst.dump();
  newTarget->dump();
  oldTarget->dump();
  inst.getOperand(0)->dump();

  errs() << "\n------- The code -------\n";
  inst.getParent()->getParent()->dump();
  errs() << "\n-------- Done showing the code ----------\n";
#endif

  StoreInst* storeInst = &inst;
  Value *valueOp = storeInst->getValueOperand();
  StoreInst* newStore UNUSED = NULL;
  Value *destination = storeInst->getOperand(1);

  if (dyn_cast<PointerType>(newType)) {

    // newTarget could be the source or destination. This is just handling destination, not source.
    
    if (valueOp == oldTarget) {
      BitCastInst *bitCast = new BitCastInst(newTarget, oldType, "", storeInst);
      newStore = new StoreInst(bitCast, destination, false, alignment, storeInst);
    } else {  

      if (dyn_cast<PointerType>(valueOp->getType())) {
	BitCastInst *bitCast = new BitCastInst(valueOp, newType, "", storeInst);
	newStore = new StoreInst(bitCast, newTarget, false, alignment, storeInst);
      }
      else {
	// new
	Type *elementType = getElementType(newType);
	if (newType->getTypeID() > oldType->getTypeID()) {
	  FPExtInst *ext = new FPExtInst(valueOp, elementType, "", storeInst);
	  newStore = new StoreInst(ext, newTarget, false, alignment, storeInst);
	}
	else {
	  FPTruncInst *fptrunc = new FPTruncInst(valueOp, elementType, "", storeInst);
	  newStore = new StoreInst(fptrunc, newTarget, false, alignment, storeInst);
	}
      }
    }
  } else {
    if (ConstantFP *constant = dyn_cast<ConstantFP>(valueOp)) {

      CastInst *fpOperand = CastInst::CreateFPCast(constant, newType, "", storeInst);
      newStore = new StoreInst(fpOperand, newTarget, false, alignment, storeInst);
    }
    else { // a register?

      if (newType->getTypeID() > oldType->getTypeID()) {
        FPExtInst *ext = new FPExtInst(valueOp, newType, "", storeInst);
        newStore = new StoreInst(ext, newTarget, false, alignment, storeInst);
      }
      else {
        FPTruncInst *fptrunc = new FPTruncInst(valueOp, newType, "", storeInst);
        newStore = new StoreInst(fptrunc, newTarget, false, alignment, storeInst);
      }
    }
  }

#ifdef DEBUG
  if (newStore) {
    errs() << "\nNew: ";
    newStore->dump();
  }
  else {
    errs() << "the new store is NULL\n";
  }
#endif

  // the old target is not erased
  return false;
}


bool Transformer::transform(BitCastInst& inst, Value* newTarget, Value* oldTarget, Type* newType, Type* oldType, unsigned alignment) {
  errs() << "Transforming BITCASTINST\n";

  BitCastInst* oldBitcast = &inst;
  BitCastInst* newBitcast = NULL;

#ifdef DEBUG
  errs() << "Old Bitcast: ";
  oldBitcast->dump();
#endif

  if (oldBitcast->getType() != newType) {
    newBitcast = new BitCastInst(newTarget, inst.getType(), "", oldBitcast);
    oldBitcast->replaceAllUsesWith(newBitcast);

    // call transformer on each of the uses of this new bitcast instruction
    vector<Instruction*> erase;
    Value::use_iterator it = newBitcast->use_begin();
    for(; it != newBitcast->use_end(); it++) {
#ifdef DEBUG
      errs() << "\t";
      it->dump();
      errs() << "\n";
#endif
      bool is_erased = Transformer::transform(it, newTarget, oldTarget, newType, oldType, alignment);
      if (!is_erased)
	erase.push_back(dyn_cast<Instruction>(*it));
    } 

    // erasing old instructions
    for (unsigned int i = 0; i < erase.size(); i++) {
      erase[i]->eraseFromParent();
    }

  }
  else {
    // there is no longer need for bitcast
    oldBitcast->replaceAllUsesWith(newTarget);
  }

#ifdef DEBUG
  errs() << "New Bitcast: ";
  newTarget->dump();
#endif

  return false;
}


bool Transformer::transform(GetElementPtrInst& inst, Value* newTarget, Value* oldTarget, Type* newType, Type* oldType, unsigned alignment) {

  GetElementPtrInst* oldGetElementPtr = &inst;

#ifdef DEBUG
  errs() << "Transforming GETELEMENTPTRINST " << alignment << "\n";
  oldGetElementPtr->dump();
#endif

  vector<Instruction*> erase;
  std::vector<llvm::Value*> indices;
  for (unsigned i = 1; i < oldGetElementPtr->getNumOperands(); i++) {
    indices.push_back(oldGetElementPtr->getOperand(i));
  }
  ArrayRef<llvm::Value*> *arrayRef = new ArrayRef<llvm::Value*>(indices);

  GetElementPtrInst* newGetElementPtr = GetElementPtrInst::CreateInBounds(newTarget, *arrayRef, "", oldGetElementPtr);

  // go through uses, targets: LoadInst and CallInst (for mm.set)
  Value::use_iterator it = oldGetElementPtr->use_begin();
  for(; it != oldGetElementPtr->use_end(); it++) {
#ifdef DEBUG
    errs() << "\tA use of getElementPtr to be replaced:\n";
    it->dump();
    errs() << "\n";
#endif

    newTarget = newGetElementPtr;
    bool is_erased = false;
    if (dyn_cast<ArrayType>(newType) || dyn_cast<PointerType>(newType)) {
      Type* temp_newType = getElementType(newType);
      Type* temp_oldType = getElementType(oldType);
      is_erased = Transformer::transform(it, newTarget, oldTarget, temp_newType, temp_oldType, alignment);
    }
    else if (CallInst* callInst = dyn_cast<CallInst>(*it)) {
      errs() << "====a call from getElementPtr\n";
      BitCastInst *bitCast = new BitCastInst(newTarget, oldGetElementPtr->getType(), "", callInst);
      bitCast->dump();
      for (unsigned i = 0; i < callInst->getNumOperands(); i++) {
	if (callInst->getOperand(i) == oldGetElementPtr)
	  callInst->setArgOperand(i, bitCast);
      }
    }
    else {
      if (newType->getTypeID() != oldType->getTypeID()) {
	is_erased = Transformer::transform(it, newTarget, oldTarget, newType, oldType, alignment);
      }
      else {
	// no further changes, but make sure we don't delete instruction!
	is_erased = true;
      }
    }
    if (!is_erased) {
      erase.push_back(dyn_cast<Instruction>(*it));
    }
  }


  // erasing old instructions
  for (unsigned int i = 0; i < erase.size(); i++) {
    erase[i]->eraseFromParent();
  }
    
  // for any uses left, replace them with bitcast instruction
  if (dyn_cast<ArrayType>(newType) || dyn_cast<PointerType>(newType)) {
    BitCastInst *bitCast = new BitCastInst(newGetElementPtr, oldGetElementPtr->getType(), "", oldGetElementPtr);
    oldGetElementPtr->replaceAllUsesWith(bitCast);  
  }
  else {
    // new struct case
    oldGetElementPtr->replaceAllUsesWith(newGetElementPtr);
  }

  // the old target was not erased
  return false;
}


bool Transformer::transform(CallInst& inst, Value* newTarget UNUSED, Value* oldTarget UNUSED, Type* newType, Type* oldType UNUSED, unsigned alignment) {
#ifdef DEBUG
  errs() << "Transforming CALLINST\n";
  errs() << "\nNew type:";
  newType->dump();
  errs() << "\nOld type:";
  oldType->dump();
  errs() << "\nNew target:";
  newTarget->dump();
  errs() << "\n";
  oldTarget->dump();
#endif

  CallInst* oldCall = &inst;
  if (oldCall->getCalledFunction()->getName() == "llvm.memcpy.p0i8.p0i8.i64") {
    // only when declaring a constant array
    // we need to create a new constant array
    Constant *constant = dyn_cast<Constant>(oldCall->getOperand(1)->stripPointerCasts());
    GlobalValue *global = dyn_cast<GlobalValue>(constant);

    ConstantArray * constantArray = dyn_cast<ConstantArray>(constant->getOperand(0));
    unsigned numElements = constantArray->getType()->getNumElements();
    Type *type = constant->getType();

    if (PointerType* pointer = dyn_cast<PointerType>(type)) {
      if (ArrayType* array = dyn_cast<ArrayType>(pointer->getElementType())) {
	if (Type* oldElem = array->getElementType()) {
	  if (Type* newElem = (dyn_cast<ArrayType>(newType))->getElementType()) {
	    if (oldElem->getTypeID() != newElem->getTypeID()) {

	      // creating new array
	      ArrayType* newArrayType = ArrayType::get(newElem, numElements);
	      std::vector<llvm::Constant*> arrayElements;

	      for(unsigned i = 0; i < numElements; i++) {

		if (ConstantFP *oldConstant = dyn_cast<ConstantFP>(constantArray->getOperand(i))) {

		  static llvm::LLVMContext& global = llvm::getGlobalContext();
		  ConstantFP *newConstant = NULL;

		  if (oldElem->getTypeID() == Type::DoubleTyID && newElem->getTypeID() == Type::FloatTyID) {
		    float fconstant = (float)(oldConstant->getValueAPF().convertToDouble());
		    APFloat newAPFloat(fconstant);
		    newConstant = ConstantFP::get(global, newAPFloat);
		  }
		  else if (oldElem->getTypeID() == Type::FloatTyID && newElem->getTypeID() == Type::DoubleTyID) {
		    double fconstant = (double)(oldConstant->getValueAPF().convertToFloat());
		    //fprintf(stderr, "---%1.15Le\n", (long double)fconstant);
		    APFloat newAPFloat(fconstant);
		    newConstant = ConstantFP::get(global, newAPFloat);
		  }
		  else {
		    errs() << "WARNING: Unhandled type when creating constant array\n";
		    exit(1);
		  }
		  // adding element to the new constant array
		  arrayElements.push_back(newConstant);
		}
	      }

	      ArrayRef<llvm::Constant*> *arrayRef = new ArrayRef<llvm::Constant*>(arrayElements);	
	      Constant* newConstantArray = ConstantArray::get(newArrayType, *arrayRef);

	      // creating new global variable
	      Module *module = oldCall->getCalledFunction()->getParent();
	      GlobalVariable *newGlobal = new GlobalVariable(*module, newConstantArray->getType(), true, GlobalValue::PrivateLinkage, newConstantArray, "");
	      newGlobal->setUnnamedAddr(true);
	      newGlobal->setAlignment(alignment);

	      // taking name from old global
	      newGlobal->takeName(constant);

	      // replacing all uses
	      constant->replaceAllUsesWith(newGlobal);

	      // changing other arguments
	      int size = 0;
	      switch(newElem->getTypeID()) {
	      case Type::FloatTyID:
		size = 4;
		break;
	      case Type::DoubleTyID:
		size = 8;
		break;
	      default:
		size = 16;
		break;
	      }

	      oldCall->setArgOperand(2, getInt64(numElements * size));
	      oldCall->setArgOperand(3, getInt32(alignment));
	      
	      // deleting global
	      global->eraseFromParent();
	    }
	  }
	}
      }
    }
  }
  else {
#ifdef DEBUG
    errs() << "CALLINST: Nothing done?\n";
#endif
  }

  // the old target was erased
  return true;
}
