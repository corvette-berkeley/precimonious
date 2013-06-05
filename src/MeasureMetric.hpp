#ifndef MEASURE_METRIC_GUARD
#define MEASURE_METRIC_GUARD 1

#include <llvm/Analysis/DebugInfo.h>
#include <llvm/Pass.h>
#include <llvm/Module.h>

#include <map>
#include <string>

namespace llvm {
	class Type;
}

using namespace std;
using namespace llvm;

class MeasureMetric : public ModulePass {
	
	public:
		MeasureMetric() : ModulePass(ID) {}

		void doInitialization(Module &M);

		virtual void getAnalysisUsage(AnalysisUsage &AU) const;

		virtual bool runOnModule(Module &M);

		bool runOnFunction(Function &F);

		void increaseCounter(Type* type, map<string, int> &counter);

		void printCounter(map<string, int> &counter);

		void doFinalization();

		static char ID; // Pass identification, replacement for typeid

	private:
		DebugInfoFinder debugInfo;

		int score; // total score

		int castInstCount; // Counter of cast isntructions

		map<string, int> arithOp; // Counter of airthmetic operations 

		map<string, int> cmpOp; // Counter of comparision operations

		map<string, int> loadOp; // Counter of load instructions

		map<string, int> storeOp; // Counter of store instructions

};

#endif
