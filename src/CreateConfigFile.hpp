#ifndef CREATE_CONFIG_FILE_GUARD
#define CREATE_CONFIG_FILE_GUARD 1

#include <llvm/Pass.h>

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

typedef vector<Value*> Variables;


class CreateConfigFile : public ModulePass {
  
public:
  CreateConfigFile() : ModulePass(ID) {}
  
  virtual bool runOnModule(Module &module);
  
  bool runOnFunction(Function &function, raw_fd_ostream &outfile, bool &first);

  bool doInitialization(Module &);

  void findFunctionCalls(Function &function, raw_fd_ostream &outfile, bool &first);

  void findGlobalVariables(Module &module, raw_fd_ostream &outfile, bool &first);

  void findLocalVariables(Function &function, raw_fd_ostream &outfile, bool &first);
  
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

#endif // CREATE_CONFIG_FILE_GUARD
