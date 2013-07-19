#ifndef PARSE_CONFIG
#define PARSE_CONFIG

#include "Change.hpp"
#include "StrChange.hpp"
#include "config_parser.hpp"
#include <llvm/Analysis/DebugInfo.h>
#include <llvm/Pass.h>

#include <map>
#include <utility>
#include <vector>

using namespace std;
using namespace llvm;

typedef vector<Change*> Changes;
enum ChangeType {GLOBALVAR, LOCALVAR, OP, CALL};


class ParseConfig : public ModulePass {

	public: 
		ParseConfig() : ModulePass(ID) {}

		/*
		 * Get the set of changes.
		 *
		 * This maps each of the four objects - globalVar,
		 * localVar, op and call - to the set of changes.
		 */
		map<ChangeType, Changes>& getChanges();

		bool doInitialization(Module &M);

		virtual bool runOnModule(Module &M);

		bool runOnFunction(Function &f);

		virtual void getAnalysisUsage(AnalysisUsage &AU) const;

		static char ID; // Pass identification, replacement for typeid

	private:
		DebugInfoFinder debugInfo;  

		void updateChanges(string, Value*, LLVMContext&);

                static Value* findAlloca(Value*, Function*);

		map<string, StrChange*> types;

		map<ChangeType, Changes> changes;

};

#endif // PARSE_CONFIG
