#ifndef CREATE_CALL_DEPENDENCY
#define CREATE_CALL_DEPENDENCY

#include <llvm/Pass.h>
#include <llvm/Analysis/DebugInfo.h>

#include <set>
#include <map>

using namespace llvm;
using namespace std;

class CreateCallDependency : public ModulePass {
	
	public: 
		CreateCallDependency() : ModulePass(ID) {}

		bool doInitialization(Module &M);

		virtual bool runOnModule(Module &M);

		virtual void getAnalysisUsage(AnalysisUsage &AU) const;

		void findDependency(string call);

		set<string> findCalledFunctions(string call);

    void findUsedGlobalVar(Module &M);

		void doFinalization();

		static char ID; // Pass identification, replacement for typeid

	private:
		DebugInfoFinder debugInfo;

		set<string> calls;

    set<string> globalVars;

		map<string, Function*> nameToCall;
};

#endif // CREATE_CALL_DEPENDENCY
