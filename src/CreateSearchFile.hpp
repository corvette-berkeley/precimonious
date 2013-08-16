#ifndef CREATE_SEARCH_FILE_GUARD
#define CREATE_SEARCH_FILE_GUARD 1

#include <llvm/Pass.h>
#include <llvm/Support/CommandLine.h>

#include <map>
#include <set>
#include <vector>

namespace llvm {
  class GlobalVariable;
  class raw_fd_ostream;
  class Type;
  class Value;
}

using namespace std;
using namespace llvm;
extern cl::opt<string> ExcludedFunctionsFileName;
extern cl::opt<string> IncludedFunctionsFileName;
extern cl::opt<string> IncludedGlobalVarsFileName;
extern cl::opt<string> ExcludedLocalVarsFileName;
extern cl::opt<bool> ListOperators;
extern cl::opt<bool> ListFunctions;
extern cl::opt<bool> OnlyScalars;
extern cl::opt<bool> OnlyArrays;
extern cl::opt<string> FileName;

typedef vector<Value*> Variables;


class CreateSearchFile : public ModulePass {
  
public:
  CreateSearchFile() : ModulePass(ID) {}
  
  virtual bool runOnModule(Module &module);
  
  bool runOnFunction(Function &function, raw_fd_ostream &outfile, bool &first);

  bool doInitialization(Module &);

  void findFunctionCalls(Function &function, raw_fd_ostream &outfile, bool &first);

  void findGlobalVariables(Module &module, raw_fd_ostream &outfile, bool &first);

  void findLocalVariables(Function &function, raw_fd_ostream &outfile, bool &first);

  void printLocal(Function &function, raw_fd_ostream &outfile, bool &first, string name, Type *type);

  void printGlobal(raw_fd_ostream &outfile, bool &first, string name, Type *type);

  void findOperators(Function &function, raw_fd_ostream &outfile, bool &first);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const;

  const map<const Function*, Variables> &getLocals();

  static char ID; // Pass identification, replacement for typeid

private:
  set<string> excludedFunctions;

  set<string> includedFunctions;

  set<string> includedGlobalVars;

  set<string> excludedLocalVars;

  set<string> functionCalls;
};

#endif // CREATE_SEARCH_FILE_GUARD
