/*
 * TaintListener.h
 *
 *  Created on: 2016年2月17日
 *      Author: 11297
 */

#ifndef LIB_CORE_TAINTLISTENER_H_
#define LIB_CORE_TAINTLISTENER_H_

#include "../Core/ExecutionState.h"
#include "klee/Expr/Expr.h"
#include "klee/ADT/Ref.h"
#include "../Core/AddressSpace.h"
#include "../Thread/StackType.h"
#include "BitcodeListener.h"
#include "Event.h"
#include "FilterSymbolicExpr.h"
#include "../Encode/RuntimeDataManager.h"

#include <map>
#include <string>
#include <vector>

namespace llvm {
	class Type;
	class Constant;
}

namespace klee {

	class TaintListener: public BitcodeListener {
		public:
			TaintListener(Executor* executor, RuntimeDataManager* rdManager);
			virtual ~TaintListener();

			void beforeRunMethodAsMain(ExecutionState &initialState);
			void beforeExecuteInstruction(ExecutionState &state, KInstruction *ki);
			void afterExecuteInstruction(ExecutionState &state, KInstruction *ki);
			void afterRunMethodAsMain(ExecutionState &state);
			void executionFailed(ExecutionState &state, KInstruction *ki);

		private:
			Executor* executor;
			Event* currentEvent;
			FilterSymbolicExpr filter;

		private:

			//add by hy
			ref<Expr> manualMakeTaintSymbolic(ExecutionState& state, std::string name, unsigned size);
			void manualMakeTaint(ref<Expr> value, bool isTaint);
			ref<Expr> readExpr(ExecutionState &state, ref<Expr> address, Expr::Width size);

	};

}

#endif /* LIB_CORE_TAINTLISTENER_H_ */
