#include "CreateConfigFile.hpp"
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

cl::opt<string> FileName("filename", cl::value_desc("filename"), cl::desc("The file name"), cl::init("file.json"));
cl::opt<string> ExcludedFunctionsFileName("exclude", cl::value_desc("filename"), cl::desc("List of functions to exclude (if in dependencies)"), cl::init("exclude.txt"));
cl::opt<string> IncludedFunctionsFileName("include", cl::value_desc("filename"), cl::desc("List of functions to include (dependencies)"), cl::init("include.txt"));
cl::opt<string> IncludedGlobalVarsFileName("include_global_vars", cl::value_desc("filename"), cl::desc("List of global variables to include"), cl::init("include_global.txt"));
cl::opt<string> ExcludedLocalVarsFileName("exclude_local_vars", cl::value_desc("filename"), cl::desc("List of local variables to exclude"), cl::init("exclude_local.txt"));
cl::opt<bool> PythonFormat("pformat", cl::value_desc("flag"), cl::desc("Use python format"), cl::init(false));
cl::opt<bool> ListOperators("ops", cl::value_desc("flag"), cl::desc("Print operators"), cl::init(false));
cl::opt<bool> ListFunctions("funs", cl::value_desc("flag"), cl::desc("Print functions"), cl::init(false));
cl::opt<bool> OnlyScalars("only-scalars", cl::value_desc("flag"), cl::desc("Print only scalars"), cl::init(false));
cl::opt<bool> OnlyArrays("only-arrays", cl::value_desc("flag"), cl::desc("Print only arrays"), cl::init(false));

static void printDimensions(vector<unsigned> &dimensions, raw_fd_ostream &outfile) {
  for(unsigned i = 0; i < dimensions.size(); i++) {
    outfile << "[" << dimensions[i] << "]";
  }
  return;
}


static void printType(Type *type, raw_fd_ostream &outfile) {
  unsigned int typeID = type->getTypeID();
  
  switch(typeID) {
  case Type::FloatTyID: 
    outfile << "float";
    break;

  case Type::DoubleTyID: 
    outfile << "double";
    break;

  case Type::X86_FP80TyID: // long double
    outfile << "longdouble";
    break;

  case Type::PPC_FP128TyID:
    outfile << "WARNING: PPC_FP128";
    break;

  case Type::IntegerTyID:
    outfile << "int";
    break;	

  case Type::PointerTyID: {
    PointerType *pointer = dyn_cast<PointerType>(type);
    Type *elementType = pointer->getElementType();
    printType(elementType, outfile);
    outfile << "*";
    break;
  }

  case Type::StructTyID:
    outfile << "struct"; // complete type
    break;

  case Type::ArrayTyID: {    
    vector<unsigned> dimensions;
    while(ArrayType* arrayType = dyn_cast<ArrayType>(type)) {
      type = arrayType->getElementType();
      dimensions.push_back(arrayType->getNumElements());
    }
    
    if (type->isFloatingPointTy()) {
      unsigned int typeID = type->getTypeID();
      
      switch(typeID) {
      case Type::FloatTyID:       
	outfile << "float";
	printDimensions(dimensions, outfile);
	break;
	
      case Type::DoubleTyID: 
	outfile << "double";
	printDimensions(dimensions, outfile);
	break;
	  
      case Type::X86_FP80TyID:
	outfile << "longdouble";
	printDimensions(dimensions, outfile);
	break;
	
      default:
	outfile << "\"WARNING: array of unrecognized type\",\n";
	break;
      }
    }
    else {
      outfile <<  *type;
      printDimensions(dimensions, outfile);
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
  if (MDNode *node = inst.getMetadata("corvette.inst.id")) {
    if (Value *value = node->getOperand(0)) {
      MDString *mdstring = cast<MDString>(value);
      id = mdstring->getString();
    }
  }
  else {
    errs() << "WARNING: Did not find metadata\n";
  }
  return id;
}


static bool isFPArray(Type *type) {
  if (ArrayType *array = dyn_cast<ArrayType>(type)) {
    type = array->getElementType();
    if (type->isFloatingPointTy()) {
      return true;
    }
    else {
      return isFPArray(type);
    }
  }
  else if (PointerType *pointer = dyn_cast<PointerType>(type)) {
    type = pointer->getElementType();
    if (type->isFloatingPointTy()) {
      return true;
    }
    else {
      return isFPArray(type);
    }
  }
  return false;
}


void CreateConfigFile::findGlobalVariables(Module &module, raw_fd_ostream &outfile, bool &first) {

  for (Module::global_iterator it = module.global_begin(); it != module.global_end(); it++) {
    Value *value = it;
    if (GlobalVariable *global = dyn_cast<GlobalVariable>(value)) {
      string name = global->getName();

      if (includedGlobalVars.find(name) != includedGlobalVars.end()) {
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
	    
	    outfile << "\t";
	    if (PythonFormat) {
	      outfile << "{";
	    }
	    
	    outfile << "\"globalVar\": {\n";
	    outfile << "\t\t\"name\": \"" << global->getName() << "\",\n";
	    outfile << "\t\t\"type\": ";
	    
	    outfile << "\"";
	    printType(elementType, outfile);
	    outfile << "\"\n";
	    
	    outfile << "\t}";	
	    if (PythonFormat) {
	      outfile << "}";
	    }
	  }
	}
      }
    }
  }
  return;
}


bool CreateConfigFile::doInitialization(Module &) {
  ifstream inFile(ExcludedFunctionsFileName.c_str());
  string name;
  
  // reading functions to ignore
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
  inFile.open (IncludedGlobalVarsFileName.c_str(), ifstream::in);
  if (!inFile) {
    errs() << "Unable to open " << IncludedGlobalVarsFileName << '\n';
    exit(1);
  }
  
  while(inFile >> name) {
    includedGlobalVars.insert(name);
  }
  inFile.close();

  // reading local variables to exclude
  // assuming unique names given by LLVM, so no need to include function name
  inFile.open (ExcludedLocalVarsFileName.c_str(), ifstream::in);
  while(inFile >> name) {
    excludedLocalVars.insert(name);
  }
  inFile.close();


  if (PythonFormat) {
    errs() << "Using python format\n";
  }
  else {
    errs() << "NOT using python format\n";
  }
  
  // populating function calls
  functionCalls.insert("log");
  //functionCalls.insert("sqrt");
  functionCalls.insert("cos"); //FT
  functionCalls.insert("sin"); //FT
  functionCalls.insert("acos"); //funarc

  return false;
}


bool CreateConfigFile::runOnModule(Module &module) {
  doInitialization(module);
  
  string errorInfo; 
  raw_fd_ostream outfile(FileName.c_str(), errorInfo);

  if (PythonFormat) {
    outfile << "{\"config\": [\n";
  }
  else {
    outfile << "{\n";
  }

  bool first = true;
  findGlobalVariables(module, outfile, first);

  for(Module::iterator f = module.begin(), fe = module.end(); f != fe; f++) {
    string name = f->getName().str();
    if (!f->isDeclaration() && (includedFunctions.find(name) != includedFunctions.end()) && (excludedFunctions.find(name) == excludedFunctions.end())) {
      runOnFunction(*f, outfile, first);
    }
  }

  if (PythonFormat) {
    outfile << "\n]}\n";
  }
  else {
    outfile << "\n}\n";
  }

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


void CreateConfigFile::findLocalVariables(Function &function, raw_fd_ostream &outfile, bool &first) {

  const ValueSymbolTable& symbolTable = function.getValueSymbolTable();
  ValueSymbolTable::const_iterator it = symbolTable.begin();
  
  for(; it != symbolTable.end(); it++) {
    Value *value = it->second;
    string name = value->getName();

    if (excludedLocalVars.find(name) == excludedLocalVars.end()) {
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
	    
	    outfile << "\t";
	    if (PythonFormat) {
	      outfile << "{";
	    }
	    
	    outfile << "\"localVar\": {\n";
	    
	    if (Instruction *i = function.getEntryBlock().getTerminator()) {
	      if (MDNode *node = i->getMetadata("dbg")) {
		DILocation loc(node);
		outfile << "\t\t\"file\": \"" << loc.getFilename() << "\",\n";
	      }
	    }
	    
	    outfile << "\t\t\"function\": \"" << function.getName() << "\",\n";
	    outfile << "\t\t\"name\": \"" << value->getName() << "\",\n";
	    outfile << "\t\t\"type\": ";
	    
	    outfile << "\"";
	    printType(type, outfile);
	    outfile << "\"\n";
	    
	    outfile << "\t}";
	    if (PythonFormat) {
	      outfile << "}";
	    }
	  }
	}
      }
    } 
  }
 return;
}


void CreateConfigFile::findOperators(Function &function, raw_fd_ostream &outfile, bool &first) {

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

	  outfile << "\t";
	  if (PythonFormat) {
	    outfile << "{";
	  }
	  
	  outfile << "\"op\": {\n";
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
	  outfile << "\t\t\"type\": ";
	  outfile << "\"";
	  printType(binOp->getType(), outfile);
	  outfile << "\"\n";

	  outfile << "\t}";
	  if (PythonFormat) {
	    outfile << "}";
	  }
	  
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
	 
	outfile << "\t";
	if (PythonFormat) {
	  outfile << "{";
	}

	outfile << "\"op\": {\n";
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
	outfile << "\t\t\"type\": ";
	outfile << "\"";
	printType(fcmp->getOperand(0)->getType(), outfile);
	outfile << "\"\n";
	
	outfile << "\t}";
	if (PythonFormat) {
	  outfile << "}";
	}

      }
    }
  }
  return;
}


void CreateConfigFile::findFunctionCalls(Function &function, raw_fd_ostream &outfile, bool &first) {

  for(Function::iterator b = function.begin(), be = function.end(); b != be; b++) {
    for(BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {

      if (CallInst *callInst = dyn_cast<CallInst>(i)) {
        Function *callee = callInst->getCalledFunction();

	if (callee) {
          string name = callee->getName();
          if (functionCalls.find(name) != functionCalls.end()) {

	    if (first) {
	      first = false;
	    }
	    else {
	      outfile << ",\n";
	    }
	    
	    outfile << "\t";
	    if (PythonFormat) {
	      outfile << "{";
	    }
	    
	    outfile << "\"call\": {\n";
	    outfile << "\t\t\"id\": \"" << getID(*i) << "\",\n";
	    outfile << "\t\t\"function\": \"" << function.getName() << "\",\n";
	    outfile << "\t\t\"name\": \"" << name << "\",\n";
            outfile << "\t\t\"switch\": \"" << name << "\",\n";
	    outfile << "\t\t\"type\": " << "[\"double\",\"double\"]"<< "\n";
	    outfile << "\t}";
	    
	    if (PythonFormat) {
	      outfile << "}";
	    }
	  }
        }
      }
    }
  }

  return;
}

bool CreateConfigFile::runOnFunction(Function &function, raw_fd_ostream &outfile, bool &first) {
  findLocalVariables(function, outfile, first);

  if (ListOperators) {
    findOperators(function, outfile, first);
  }

  if (ListFunctions) {
    findFunctionCalls(function, outfile, first);
  }
  return false;
}


void CreateConfigFile::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<CreateIDBitcode>();
}


char CreateConfigFile::ID = 0;
static const RegisterPass<CreateConfigFile> registration("config-file", "Creating a config file with original types");

