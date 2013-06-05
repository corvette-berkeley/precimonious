#include "CreateSearchFile.hpp"
#include "CreateIDBitcode.hpp"

#include <llvm/Analysis/DebugInfo.h>
#include <llvm/Instructions.h>
#include <llvm/Module.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ValueSymbolTable.h>

#include <fstream>
#include <sstream>

using namespace llvm;

cl::opt<bool> StartFromOriginalType("original-type", cl::value_desc("flag"), cl::desc("Start from the original types"), cl::init(false));


static void printDimensions(vector<unsigned> &dimensions, raw_fd_ostream &outfile) {
  for(unsigned i = 0; i < dimensions.size(); i++) {
    outfile << "[" << dimensions[i] << "]";
  }
  return;
}


static void printAll(string asterisk, raw_fd_ostream &outfile) {
  outfile << "[\"float" << asterisk << "\", \"double" << asterisk << "\", \"longdouble" << asterisk << "\"]\n";
  return;
}


static void printAllArray(vector<unsigned> &dimensions, raw_fd_ostream &outfile) {
  outfile << "[";
  outfile << "\"float";
  printDimensions(dimensions, outfile);
  outfile << "\", ";
  
  outfile << "\"double";
  printDimensions(dimensions, outfile);
  outfile << "\", ";
  
  outfile << "\"longdouble";
  printDimensions(dimensions, outfile);
  outfile << "\"]\n";
  return;
}


static void printType(Type *type, raw_fd_ostream &outfile) {
  unsigned int typeID = type->getTypeID();
  
  switch(typeID) {
  
  case Type::FloatTyID:
    if (StartFromOriginalType) {
      outfile << "[\"float\"]\n";
    }
    else {
      printAll("", outfile);
    }
    break;

  case Type::DoubleTyID: 
    if (StartFromOriginalType) {
      outfile << "[\"float\", \"double\"]\n";
    }
    else {
      printAll("", outfile);
    }
    break;

  case Type::X86_FP80TyID:
  case Type::PPC_FP128TyID:
    printAll("", outfile);
    break;

  case Type::IntegerTyID:
    outfile << "\"int\"\n";
    break;	

  case Type::PointerTyID: {
    PointerType *pointer = dyn_cast<PointerType>(type);
    Type *elementType = pointer->getElementType();
    unsigned int typeElementID = elementType->getTypeID();
    switch(typeElementID) {
    case Type::FloatTyID: 
      if (StartFromOriginalType) {
	outfile << "[\"float*\"]\n";
      }
      else {
	printAll("*", outfile);
      }
      break;
	
    case Type::DoubleTyID: 
      if (StartFromOriginalType) {
	outfile << "[\"float*\", \"double*\"]\n";
      }
      else {
	printAll("*", outfile);
      }
      break;
      
    case Type::X86_FP80TyID:
      printAll("*", outfile);
      break;
      
    default:
      outfile << "\"pointer\"\n";
      break;
    }
    break;
  }
   
  case Type::StructTyID:
    outfile << "\"struct\"\n"; // complete type
    break;
  
  case Type::ArrayTyID: {
    vector<unsigned> dimensions;
    while(ArrayType* arrayType = dyn_cast<ArrayType>(type)) {
      type = arrayType->getElementType();
      dimensions.push_back(arrayType->getNumElements());
    }
    
    if (type->isFloatingPointTy()) {
      unsigned int elementTypeID = type->getTypeID();
      
      switch(elementTypeID) {
      case Type::FloatTyID:
	if (StartFromOriginalType) {
	  outfile << "[";
	  outfile << "\"float";
	  printDimensions(dimensions, outfile);
	  outfile << "\"]\n";
	}
	else {
	  printAllArray(dimensions, outfile);
	}
	break;
	
      case Type::DoubleTyID: 
	if (StartFromOriginalType) {
	  outfile << "[";
	  outfile << "\"float";
	  printDimensions(dimensions, outfile);
	  outfile << "\", ";
	  
	  outfile << "\"double";
	  printDimensions(dimensions, outfile);
	  outfile << "\"]\n";
	}
	else {
	  printAllArray(dimensions, outfile);
	}
	break;
	
      case Type::X86_FP80TyID:
      case Type::PPC_FP128TyID:
	printAllArray(dimensions, outfile);
	break;
	
      default:
	// do nothing
	break;
      }
    }
    else {
      outfile << "\"" << *type;
      printDimensions(dimensions, outfile);
      outfile << "\"\n";
    }
    
    break;
  }

  default:
    errs() << "WARNING: Variable of type " << *type << "\n";
    break;
  }
  
  return;
}


static string getID(Instruction &inst) {
  string id = "";
  if (inst.hasMetadata()) {
    id = "";
  }
  /*
  if (MDNode *node = inst.getMetadata("corvette.inst.id")) {
    if (Value *value = node->getOperand(0)) {
      MDString *mdstring = cast<MDString>(value);
      id = mdstring->getString();
    }
  }
  else {
    errs() << "WARNING: Did not find metadata\n";
  }
  */
  return id;
}


static bool isFPArray(Type *type) {
  if (ArrayType *array = dyn_cast<ArrayType>(type)) {
    type = array->getElementType();
    if (type->isFloatingPointTy()) {
      return true;
    }
  }
  else if (PointerType *pointer = dyn_cast<PointerType>(type)) {
    type = pointer->getElementType();
    if (type->isFloatingPointTy()) {
      return true;
    }
  }
  return false;
}


void CreateSearchFile::findGlobalVariables(Module &module, raw_fd_ostream &outfile, bool &first) {

  for (Module::global_iterator it = module.global_begin(); it != module.global_end(); it++) {
    Value *value = it;
    if (GlobalVariable *global = dyn_cast<GlobalVariable>(value)) {
      string name = global->getName();

      if (includedVars.find(name) != includedVars.end()) {
	PointerType* pointerType = global->getType();      
	Type* elementType = pointerType->getElementType();
	
	if ((OnlyScalars && elementType->isFloatingPointTy()) ||
	    (OnlyArrays && isFPArray(elementType)) || 
	    (!OnlyScalars && !OnlyArrays)) {
	  
	  if (name.find('.') == string::npos) {
	    
	    if (first) {
	      first = false;
	    }
	    else {
	      outfile << ",\n";
	    }
	    
	    outfile << "\t{\"globalVar\": {\n";
	    outfile << "\t\t\"name\": \"" << global->getName() << "\",\n";
	    outfile << "\t\t\"type\": ";	
	    printType(elementType, outfile);
	    outfile << "\t}}";
	  }
	}
      }
    }
  }
  return;
}


bool CreateSearchFile::doInitialization(Module &) {
  ifstream inFile(ExcludedFunctionsFileName.c_str());
  string name;
  
  // reading functions to exclude
  if (!inFile) {
    errs() << "Unable to open " << ExcludedFunctionsFileName << '\n';
    exit(1);
  }
  
  while(inFile >> name) {
    excludedFunctions.insert(name);
  }
  inFile.close();
  
  // reading functions to include
  inFile.open (IncludedFunctionsFileName.c_str(), ifstream::in);
  if (!inFile) {
    errs() << "Unable to open " << IncludedFunctionsFileName << '\n';
    exit(1);
  }
  
  while(inFile >> name) {
    includedFunctions.insert(name);
  }
  inFile.close();

  // reading global variables to include
  inFile.open (IncludedVarsFileName.c_str(), ifstream::in);
  if (!inFile) {
    errs() << "Unable to open " << IncludedVarsFileName << '\n';
    exit(1);
  }
  
  while(inFile >> name) {
    includedVars.insert(name);
  }
  inFile.close();

  return false;
}


bool CreateSearchFile::runOnModule(Module &module) {
  doInitialization(module);
  
  string errorInfo; 
  raw_fd_ostream outfile(FileName.c_str(), errorInfo);
  outfile << "{\"config\": [\n";

  bool first = true;
  findGlobalVariables(module, outfile, first);

  for(Module::iterator f = module.begin(), fe = module.end(); f != fe; f++) {
    string name = f->getName().str();
    if (!f->isDeclaration()  && (includedFunctions.find(name) != includedFunctions.end()) && (excludedFunctions.find(name) == excludedFunctions.end())) {
      runOnFunction(*f, outfile, first);
    }
  }
  outfile << "\n]}\n";
  return false;
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


void CreateSearchFile::findLocalVariables(Function &function, raw_fd_ostream &outfile, bool &first) {

  const ValueSymbolTable& symbolTable = function.getValueSymbolTable();
  ValueSymbolTable::const_iterator it = symbolTable.begin();
  
  for(; it != symbolTable.end(); it++) {
    Value *value = it->second;
    Type *type = findLocalType(value);

    if (type) {

      if ((OnlyScalars && type->isFloatingPointTy()) ||
	  (OnlyArrays && isFPArray(type)) || 
	  (!OnlyScalars && !OnlyArrays)) {
	
	if (value->getName().find('.') == string::npos) {

	  if (first) {
	    first = false;
	  }
	  else {
	    outfile << ",\n";
	  }
	  
	  outfile << "\t{\"localVar\": {\n";

      /*
	  if (Instruction *i = function.getEntryBlock().getTerminator()) {
	    if (MDNode *node = i->getMetadata("dbg")) {
	      DILocation loc(node);
	      outfile << "\t\t\"file\": \"" << loc.getFilename() << "\",\n";
	    }
	  }
      */

	  outfile << "\t\t\"function\": \"" << function.getName() << "\",\n";
	  outfile << "\t\t\"name\": \"" << value->getName() << "\",\n";
	  outfile << "\t\t\"type\": ";	  
	  printType(type, outfile);
	  outfile << "\t}}";
	} 
      }
    }
  }
 return;
}


void CreateSearchFile::findOperators(Function &function, raw_fd_ostream &outfile, bool &first) {

  for(Function::iterator b = function.begin(), be = function.end(); b != be; b++) {
    for(BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {

      if (BinaryOperator *binOp = dyn_cast<BinaryOperator>(i)) {
	switch(binOp->getOpcode())
	case Instruction::FAdd:
	case Instruction::FSub:
	case Instruction::FMul:
	case Instruction::FDiv: {

	  if (first) {
	    first = false;
	  }
	  else {
	    outfile << ",\n";
	  }

	  outfile << "\t{\"op\": {\n";
	  outfile << "\t\t\"id\": \"" << getID(*i) << "\",\n";

	  /*
	  if (MDNode *node = binOp->getMetadata("dbg")) {
	    DILocation loc(node);
	    outfile << "\t\t\"file\": \"" << loc.getFilename() << "\",\n";
	    outfile << "\t\t\"line\": \"" << loc.getLineNumber() << "\",\n";
	  }
	  */

	  outfile << "\t\t\"function\": \"" << function.getName() << "\",\n";
	  outfile << "\t\t\"name\": \"" << binOp->getOpcodeName() << "\",\n";
	  outfile << "\t\t\"type\": [\"float\", \"double\", \"longdouble\"]\n";
	  outfile << "\t}}";
	  break;
	default:
	  // do nothing
	  break;
	}
      }
      else if (FCmpInst *fcmp = dyn_cast<FCmpInst>(i)){

	if (first) {
	  first = false;
	}
	else {
	  outfile << ",\n";
	}

	outfile << "\t{\"op\": {\n";
	outfile << "\t\t\"id\": \"" << getID(*i) << "\",\n";

	/*
	if (MDNode *node = fcmp->getMetadata("dbg")) {
	  DILocation loc(node);
	  outfile << "\t\t\"file\": \"" << loc.getFilename() << "\",\n";
	  outfile << "\t\t\"line\": \"" << loc.getLineNumber() << "\",\n";
	}
	*/

	outfile << "\t\t\"function\": \"" << function.getName() << "\",\n";
	outfile << "\t\t\"name\": \"" << fcmp->getOpcodeName() << "\",\n";
	outfile << "\t\t\"type\": [\"float\", \"double\", \"longdouble\"]\n";
	outfile << "\t}}";
      }
    }
  }
  return;
}


void CreateSearchFile::findFunctionCalls(Function &function/*, raw_fd_ostream &outfile*/) {

  for(Function::iterator b = function.begin(), be = function.end(); b != be; b++) {
    for(BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {

      if (CallInst *callInst = dyn_cast<CallInst>(i)) {
	Function *callee = callInst->getCalledFunction();
	if (callee && !callee->isDeclaration()) {
	  errs() << "\t\"call\": {\n";
	  errs() << "\t\t\"scope\": \"\",\n";
	  errs() << "\t\t\"name\": \"" << callee->getName() << "\",\n";
	  errs() << "\t\t\"type\": [\"PENDING\"]\n";
	  errs() << "\t},\n";
	}
      }
    }
  }

  return;
}


bool CreateSearchFile::runOnFunction(Function &function, raw_fd_ostream &outfile, bool &first) {
  findLocalVariables(function, outfile, first);

  if (ListOperators) {
    findOperators(function, outfile, first);
  }

  // findFunctionCalls(function);
  return false;
}


void CreateSearchFile::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<CreateIDBitcode>();
}


char CreateSearchFile::ID = 0;
static const RegisterPass<CreateSearchFile> registration("search-file", "Creating initial search file");

