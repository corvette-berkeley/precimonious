#ifndef OPERATORS_GUARD
#define OPERATORS_GUARD 1

#include "ParseConfig.hpp"

#include <llvm/Analysis/DebugInfo.h>
#include <llvm/Instructions.h>
#include <llvm/Pass.h>
#include <llvm/Module.h>

#include <map>
#include <vector>

namespace llvm {
	class Value;
	class BinaryOperator;
	class FCmpInst;
	class Type;
}

using namespace std;
using namespace llvm;

class Operators : public ModulePass {

	public:
		Operators() : ModulePass(ID) {}

		void doInitialization(Module &module);

		virtual void getAnalysisUsage(AnalysisUsage &AU) const;

		virtual bool runOnModule(Module &module);

		static char ID; // Pass identification, replacement for typeid

	private:
		void changeBinaryOperator(BinaryOperator* old_target, Type* new_type);

		void changeFCmpInst(FCmpInst* old_target, Type* new_type);

		DebugInfoFinder debugInfo;

		map<ChangeType, Changes> changes;
};

#endif
