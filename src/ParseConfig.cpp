#include "ParseConfig.hpp"

#include "Change.hpp"
#include "CreateIDBitcode.hpp"
#include "config_parser.hpp"
#include "vjson/json.h"
#include "vjson/block_allocator.h"
#include <string>
#include <string.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <utility>
#include <llvm/Instructions.h>
#include <llvm/Module.h>
#include <llvm/ValueSymbolTable.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>

using namespace std;
using namespace llvm;

/**
 * Get corvette.inst.id meta data.
 *
 * Every instruction has a corvette.inst.id metadata added
 * by the CreateIDBitcode pass. This metadata associates
 * each instruction accrossing the entire program with 
 * a unique id.
 */
static string getID(Instruction &inst) {
  string id = "";
  if (MDNode *node = inst.getMetadata("corvette.inst.id")) {
    if (Value *value = node->getOperand(0)) {
      MDString *mdstring = cast<MDString>(value);
      id = mdstring->getString();
    }
  }
  else {
    //errs() << "WARNING: Did not find metadata\n";
  }
  return id;
}

/**
 * Declaring json-config as an option to pass configuration 
 * input file to this pass.
 */
cl::opt<string> JsonConfigFileName("json-config", cl::value_desc("filename"), cl::desc("Configuration input file"), cl::init("config.json"));

/*
 * Get the map of type changes. 
 *
 * This maps each of the four objects - globalVar,
 * localVar, op and call - to the set of changes.
 */
map<ChangeType, Changes> & ParseConfig::getChanges() {
  return changes;
}


/**
 * Pass initialization.
 */
bool ParseConfig::doInitialization(Module &M) {
  debugInfo.processModule(M);

  const char* fileName = JsonConfigFileName.c_str();
  types = parse_config(fileName);

  Changes globalVarVec;
  changes[GLOBALVAR] = globalVarVec;

  Changes localVarVec;
  changes[LOCALVAR] = localVarVec;

  Changes opVec;
  changes[OP] = opVec;

  Changes callVec;
  changes[CALL] = callVec;

  return true;
}


bool ParseConfig::runOnModule(Module &M) {
  doInitialization(M);

  Module::global_iterator it = M.global_begin();
  LLVMContext& context = M.getContext();
  for(; it != M.global_end(); it++) {
    Value *value = it;
    if (GlobalVariable *global = dyn_cast<GlobalVariable>(value)) {
      string varId = global->getName();
      updateChanges(varId, value, context);
    }
  }

  for(Module::iterator f = M.begin(), fe = M.end(); f != fe; f++) {
    if (!f->isDeclaration()) {
      runOnFunction(*f);
    }
  }

  for (map<ChangeType, Changes>::iterator im = changes.begin(); im != changes.end(); im++) {
    ChangeType changeType = im->first;
    Changes changeVector = im->second;

    errs() << changeType << ":\n";
    for (Changes::iterator ic = changeVector.begin(); ic != changeVector.end(); ic++) {
      Change *change = *ic;
      Value *value = change->getValue();
      if (BinaryOperator *op = dyn_cast<BinaryOperator>(value)) {
        errs() << "\tname: " << op->getOpcodeName() << ", ";
      }
      else if (FCmpInst *fcmp = dyn_cast<FCmpInst>(value)) {
        errs() << "\tname: " << fcmp->getOpcodeName() << ", ";
      }
      else {
        errs() << "\tname: " << value->getName() << ", ";
      }
      errs() << "type: "; 
      change->getType().at(0)->dump(); 
      errs() << "\n";
    }
  }

  return false;
}


bool ParseConfig::runOnFunction(Function &f) {
  // local variables
  string functionName = f.getName();
  LLVMContext& context = f.getContext();
  const ValueSymbolTable& symbolTable = f.getValueSymbolTable();
  ValueSymbolTable::const_iterator it = symbolTable.begin();

  for(; it != symbolTable.end(); it++) {
    Value *value = it->second;
    if (dyn_cast<Argument>(value)) {
      value = findAlloca(value, &f);
    }

    string varIdStr = "";
    //varIdStr += value->getName();
    varIdStr += it->second->getName();
    varIdStr += "@";
    varIdStr += functionName;
    updateChanges(varIdStr, value, context);
  }

  // operations and calls
  for (Function::iterator b = f.begin(), be = f.end(); b != be; b++) {
    for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {
      Instruction *inst = i;
      string id = getID(*i);
      updateChanges(id, inst, context);
    }
  }

  return false;
}

static Type* constructStruct(Value *value, unsigned int fieldToChange, Type *fieldType) {

  Type *type = value->getType();
  StructType *newStructType = NULL;

  if (PointerType *pointerType = dyn_cast<PointerType>(type)) {
    if (StructType *oldStructType = dyn_cast<StructType>(pointerType->getElementType())) {

      vector<Type*> fields;
      
      for(unsigned int i = 0; i < oldStructType->getNumElements(); i++) {
	if (i != fieldToChange) {
	  fields.push_back(oldStructType->getElementType(i));
	}
	else {
	  // replace old field type with new type
	  fields.push_back(fieldType);
	}
      }

      ArrayRef<Type*> *arrayRefFields = new ArrayRef<Type*>(fields);
      newStructType = StructType::create(*arrayRefFields, oldStructType->getName());
    }
  }
  return newStructType;
}


void ParseConfig::updateChanges(string id, Value* value, LLVMContext& context) {
  map<string, StrChange*>::iterator typeIt;
  typeIt = types.find(id);
  if (typeIt != types.end()) {
    string changeType = typeIt->second->getClassification();
    string typeStrs = typeIt->second->getTypes();
    int field = typeIt->second->getField();
    Types type;
    // split type string
    // (in case of function there are two or more types,
    // in case of array there are size info)
    istringstream iss(typeStrs);

    do {
      string typeStr;
      iss >> typeStr;

      if (typeStr.compare("float") == 0) {
        Type *aType = Type::getFloatTy(context);
        type.push_back(aType);
      } else if (typeStr.compare("double") == 0) {
        Type *aType = Type::getDoubleTy(context);
        type.push_back(aType);
      } else if (typeStr.compare("longdouble") == 0) {
        Type *aType = Type::getX86_FP80Ty(context);
        type.push_back(aType);
      } else if (typeStr.compare(0, 6, "float[") == 0) {
        bool first = true;
        size_t openBracketPos, closeBracketPos; 
        int size;
        Type *aType;
        while ((openBracketPos = typeStr.find_last_of('[')) != string::npos) {
          closeBracketPos = typeStr.find_last_of(']');
          size = atoi(typeStr.substr(openBracketPos+1, closeBracketPos - openBracketPos - 1).c_str());
          if (first) {
            aType = ArrayType::get(Type::getFloatTy(context), size);
            first = false;
          } else {
            aType = ArrayType::get(aType, size);
          }
          typeStr = typeStr.substr(0, openBracketPos);
        }
        type.push_back(aType);
      } else if (typeStr.compare(0, 7, "double[") == 0) {
        bool first = true;
        size_t openBracketPos, closeBracketPos;
        int size;
        Type *aType;
        while ((openBracketPos = typeStr.find_last_of('[')) != string::npos) {
          closeBracketPos = typeStr.find_last_of(']');
          size = atoi(typeStr.substr(openBracketPos+1, closeBracketPos - openBracketPos - 1).c_str());
          if (first) {
            aType = ArrayType::get(Type::getDoubleTy(context), size);
            first = false;
          } else {
            aType = ArrayType::get(aType, size);
          }
          typeStr = typeStr.substr(0, openBracketPos);
        }
        type.push_back(aType);
      } else if (typeStr.compare(0, 11, "longdouble[") == 0) {
        bool first = true;
        size_t openBracketPos, closeBracketPos; 
        int size;
        Type *aType;
        while ((openBracketPos = typeStr.find_last_of('[')) != string::npos) {
          closeBracketPos = typeStr.find_last_of(']');
          size = atoi(typeStr.substr(openBracketPos+1, closeBracketPos - openBracketPos - 1).c_str());
          if (first) {
            aType = ArrayType::get(Type::getX86_FP80Ty(context), size);
            first = false;
          } else {
            aType = ArrayType::get(aType, size);
          }
          typeStr = typeStr.substr(0, openBracketPos);
        }
        type.push_back(aType);
      } else if (typeStr.compare(0, 6, "float*") == 0) {
        Type *aType = PointerType::getUnqual(Type::getFloatTy(context));
        type.push_back(aType);
      } else if (typeStr.compare(0, 7, "double*") == 0) {
        Type *aType = PointerType::getUnqual(Type::getDoubleTy(context));
        type.push_back(aType);
      } else if (typeStr.compare(0, 11, "longdouble*") == 0) {
        Type *aType = PointerType::getUnqual(Type::getX86_FP80Ty(context));
        type.push_back(aType);
      }
    } while (iss);

    // SPECIAL CASE: structs
    if (field >= 0) {
      type[0] = constructStruct(value, field, type[0]); // first element?
    }

    if (type.size() > 0) { // todo: this fix does not work in case of function call
      if (changeType.compare("globalVar") == 0) {
        changes.find(GLOBALVAR)->second.push_back(new Change(type, value, field));
      } else if (changeType.compare("localVar") == 0) {
        changes.find(LOCALVAR)->second.push_back(new Change(type, value, field));
      } else if (changeType.compare("op") == 0) {
        changes.find(OP)->second.push_back(new Change(type, value));
      } else if (changeType.compare("call") == 0) {
        changes.find(CALL)->second.push_back(new Change(type, value));
      }
    }
  }
} 


Value* ParseConfig::findAlloca(Value *value, Function *function) {

  for(Function::iterator b = function->begin(), be = function->end(); b != be; b++) {
    for(BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {

      if (StoreInst *store = dyn_cast<StoreInst>(i)) {
        Value *op1 = store->getOperand(0);
        Value *op2 = store->getOperand(1);
        if (op1 == value) {
	  op2->takeName(value);
          return op2;
        }
      }
    }
  }
  return value;
}


void ParseConfig::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  //AU.addRequired<CreateIDBitcode>();
}

char ParseConfig::ID = 0;

static const RegisterPass<ParseConfig> registration("parse-config", "Parse config file");
