//===-- ExecutionState.cpp ------------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "ExecutionState.h"

#include "Memory.h"

#include "klee/Expr/Expr.h"
#include "klee/Module/Cell.h"
#include "klee/Module/InstructionInfoTable.h"
#include "klee/Module/KInstruction.h"
#include "klee/Module/KModule.h"
#include "klee/Support/Casting.h"
#include "klee/Support/OptionCategories.h"

#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DebugInfoMetadata.h"

#include "klee/Thread/StackType.h"
#include "klee/Thread/Thread.h"
#include "klee/Thread/ThreadScheduler.h"

#include <cassert>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <stdarg.h>

using namespace llvm;
using namespace klee;

namespace {
cl::opt<bool> DebugLogStateMerge(
    "debug-log-state-merge", cl::init(false),
    cl::desc("Debug information for underlying state merging (default=false)"),
    cl::cat(MergeCat));
}

/***/

std::uint32_t ExecutionState::nextID = 1;

/***/

ExecutionState::ExecutionState(KFunction *kf)
    : depth(0), ptreeNode(nullptr), steppedInstructions(0), instsSinceCovNew(0),
      coveredNew(false), forkDisabled(false), nextThreadId(1), mutexManager(),
      condManager() {
  condManager.setMutexManager(&mutexManager);
  threadScheduler = getThreadSchedulerByType(ThreadScheduler::FIFS);
  Thread *thread = new Thread(getNextThreadId(), NULL, kf, &addressSpace);
  currentStack = thread->stack;
  threadList.addThread(thread);
  threadScheduler->addItem(thread);
  currentThread = thread;
}

ExecutionState::ExecutionState(KFunction *kf, Prefix *prefix)
    : depth(0), ptreeNode(nullptr), steppedInstructions(0), instsSinceCovNew(0),
      coveredNew(false), forkDisabled(false), nextThreadId(1), mutexManager(),
      condManager() {

  condManager.setMutexManager(&mutexManager);
  threadScheduler =
      new GuidedThreadScheduler(this, ThreadScheduler::FIFS, prefix);
  Thread *thread = new Thread(getNextThreadId(), NULL, kf, &addressSpace);
  currentStack = thread->stack;
  threadList.addThread(thread);
  threadScheduler->addItem(thread);
  currentThread = thread;
}

ExecutionState::~ExecutionState() {
  for (const auto &cur_mergehandler: openMergeStack){
    cur_mergehandler->removeOpenState(this);
  }

  for (ThreadList::iterator ti = threadList.begin(), te = threadList.end();
       ti != te; ti++) {
    delete *ti;
  }
  delete threadScheduler;
}

ExecutionState::ExecutionState(const ExecutionState& state):
    depth(state.depth),
    addressSpace(state.addressSpace),
    constraints(state.constraints),
    pathOS(state.pathOS),
    symPathOS(state.symPathOS),
    coveredLines(state.coveredLines),
    symbolics(state.symbolics),
    arrayNames(state.arrayNames),
    openMergeStack(state.openMergeStack),
    steppedInstructions(state.steppedInstructions),
    instsSinceCovNew(state.instsSinceCovNew),
    unwindingInformation(state.unwindingInformation
                             ? state.unwindingInformation->clone()
                             : nullptr),
    coveredNew(state.coveredNew),
    forkDisabled(state.forkDisabled) {
  for (const auto &cur_mergehandler: openMergeStack)
    cur_mergehandler->addOpenState(this);

  for (ThreadList::iterator ti = state.threadList.begin(),
                            te = state.threadList.end();
       ti != te; ti++) {
    Thread *thread = new Thread(**ti, &addressSpace);
    threadList.addThread(thread);
  }
  currentThread = findThreadById(state.currentThread->threadId);
  std::map<unsigned, Thread *> unfinishedThread =
      threadList.getAllUnfinishedThreads();
  threadScheduler = new FIFSThreadScheduler(
      *(FIFSThreadScheduler *)(state.threadScheduler), unfinishedThread);
}

ExecutionState *ExecutionState::branch() {
  depth++;

  auto *falseState = new ExecutionState(*this);
  falseState->setID();
  falseState->coveredNew = false;
  falseState->coveredLines.clear();

  return falseState;
}

void ExecutionState::addSymbolic(const MemoryObject *mo, const Array *array) {
  symbolics.emplace_back(ref<const MemoryObject>(mo), array);
}

/**/

llvm::raw_ostream &klee::operator<<(llvm::raw_ostream &os, const MemoryMap &mm) {
  os << "{";
  MemoryMap::iterator it = mm.begin();
  MemoryMap::iterator ie = mm.end();
  if (it!=ie) {
    os << "MO" << it->first->id << ":" << it->second.get();
    for (++it; it!=ie; ++it)
      os << ", MO" << it->first->id << ":" << it->second.get();
  }
  os << "}";
  return os;
}

bool ExecutionState::merge(const ExecutionState &b) {
  if (DebugLogStateMerge)
    llvm::errs() << "-- attempting merge of A:" << this << " with B:" << &b
                 << "--\n";
  if (currentThread->pc != b.currentThread->pc)
    return false;

  // XXX is it even possible for these to differ? does it matter? probably
  // implies difference in object states?

  if (symbolics != b.symbolics)
    return false;

  {
    std::vector<StackFrame>::const_iterator itA = currentStack->realStack.begin();
    std::vector<StackFrame>::const_iterator itB = b.currentStack->realStack.begin();
    while (itA!=currentStack->realStack.end() && itB!=b.currentStack->realStack.end()) {
      // XXX vaargs?
      if (itA->caller!=itB->caller || itA->kf!=itB->kf)
        return false;
      ++itA;
      ++itB;
    }
    if (itA!=currentStack->realStack.end() || itB!=b.currentStack->realStack.end())
      return false;
  }

  std::set< ref<Expr> > aConstraints(constraints.begin(), constraints.end());
  std::set< ref<Expr> > bConstraints(b.constraints.begin(), 
                                     b.constraints.end());
  std::set< ref<Expr> > commonConstraints, aSuffix, bSuffix;
  std::set_intersection(aConstraints.begin(), aConstraints.end(),
                        bConstraints.begin(), bConstraints.end(),
                        std::inserter(commonConstraints, commonConstraints.begin()));
  std::set_difference(aConstraints.begin(), aConstraints.end(),
                      commonConstraints.begin(), commonConstraints.end(),
                      std::inserter(aSuffix, aSuffix.end()));
  std::set_difference(bConstraints.begin(), bConstraints.end(),
                      commonConstraints.begin(), commonConstraints.end(),
                      std::inserter(bSuffix, bSuffix.end()));
  if (DebugLogStateMerge) {
    llvm::errs() << "\tconstraint prefix: [";
    for (std::set<ref<Expr> >::iterator it = commonConstraints.begin(),
                                        ie = commonConstraints.end();
         it != ie; ++it)
      llvm::errs() << *it << ", ";
    llvm::errs() << "]\n";
    llvm::errs() << "\tA suffix: [";
    for (std::set<ref<Expr> >::iterator it = aSuffix.begin(),
                                        ie = aSuffix.end();
         it != ie; ++it)
      llvm::errs() << *it << ", ";
    llvm::errs() << "]\n";
    llvm::errs() << "\tB suffix: [";
    for (std::set<ref<Expr> >::iterator it = bSuffix.begin(),
                                        ie = bSuffix.end();
         it != ie; ++it)
      llvm::errs() << *it << ", ";
    llvm::errs() << "]\n";
  }

  // We cannot merge if addresses would resolve differently in the
  // states. This means:
  // 
  // 1. Any objects created since the branch in either object must
  // have been free'd.
  //
  // 2. We cannot have free'd any pre-existing object in one state
  // and not the other

  if (DebugLogStateMerge) {
    llvm::errs() << "\tchecking object states\n";
    llvm::errs() << "A: " << addressSpace.objects << "\n";
    llvm::errs() << "B: " << b.addressSpace.objects << "\n";
  }
    
  std::set<const MemoryObject*> mutated;
  MemoryMap::iterator ai = addressSpace.objects.begin();
  MemoryMap::iterator bi = b.addressSpace.objects.begin();
  MemoryMap::iterator ae = addressSpace.objects.end();
  MemoryMap::iterator be = b.addressSpace.objects.end();
  for (; ai!=ae && bi!=be; ++ai, ++bi) {
    if (ai->first != bi->first) {
      if (DebugLogStateMerge) {
        if (ai->first < bi->first) {
          llvm::errs() << "\t\tB misses binding for: " << ai->first->id << "\n";
        } else {
          llvm::errs() << "\t\tA misses binding for: " << bi->first->id << "\n";
        }
      }
      return false;
    }
    if (ai->second.get() != bi->second.get()) {
      if (DebugLogStateMerge)
        llvm::errs() << "\t\tmutated: " << ai->first->id << "\n";
      mutated.insert(ai->first);
    }
  }
  if (ai!=ae || bi!=be) {
    if (DebugLogStateMerge)
      llvm::errs() << "\t\tmappings differ\n";
    return false;
  }
  
  // merge stack

  ref<Expr> inA = ConstantExpr::alloc(1, Expr::Bool);
  ref<Expr> inB = ConstantExpr::alloc(1, Expr::Bool);
  for (std::set< ref<Expr> >::iterator it = aSuffix.begin(), 
         ie = aSuffix.end(); it != ie; ++it)
    inA = AndExpr::create(inA, *it);
  for (std::set< ref<Expr> >::iterator it = bSuffix.begin(), 
         ie = bSuffix.end(); it != ie; ++it)
    inB = AndExpr::create(inB, *it);

  // XXX should we have a preference as to which predicate to use?
  // it seems like it can make a difference, even though logically
  // they must contradict each other and so inA => !inB

  std::vector<StackFrame>::iterator itA = currentStack->realStack.begin();
  std::vector<StackFrame>::const_iterator itB = b.currentStack->realStack.begin();
  for (; itA!=currentStack->realStack.end(); ++itA, ++itB) {
    StackFrame &af = *itA;
    const StackFrame &bf = *itB;
    for (unsigned i=0; i<af.kf->numRegisters; i++) {
      ref<Expr> &av = af.locals[i].value;
      const ref<Expr> &bv = bf.locals[i].value;
      if (!av || !bv) {
        // if one is null then by implication (we are at same pc)
        // we cannot reuse this local, so just ignore
      } else {
        av = SelectExpr::create(inA, av, bv);
      }
    }
  }

  for (std::set<const MemoryObject*>::iterator it = mutated.begin(), 
         ie = mutated.end(); it != ie; ++it) {
    const MemoryObject *mo = *it;
    const ObjectState *os = addressSpace.findObject(mo);
    const ObjectState *otherOS = b.addressSpace.findObject(mo);
    assert(os && !os->readOnly && 
           "objects mutated but not writable in merging state");
    assert(otherOS);

    ObjectState *wos = addressSpace.getWriteable(mo, os);
    for (unsigned i=0; i<mo->size; i++) {
      ref<Expr> av = wos->read8(i);
      ref<Expr> bv = otherOS->read8(i);
      wos->write(i, SelectExpr::create(inA, av, bv));
    }
  }

  constraints = ConstraintSet();

  ConstraintManager m(constraints);
  for (const auto &constraint : commonConstraints)
    m.addConstraint(constraint);
  m.addConstraint(OrExpr::create(inA, inB));

  return true;
}

void ExecutionState::dumpStack(llvm::raw_ostream &out) const {
  unsigned idx = 0;
  const KInstruction *target = currentThread->prevPC;
  for (ExecutionState::stack_ty::const_reverse_iterator
         it = currentStack->realStack.rbegin(), ie = currentStack->realStack.rend();
       it != ie; ++it) {
    const StackFrame &sf = *it;
    Function *f = sf.kf->function;
    const InstructionInfo &ii = *target->info;
    out << "\t#" << idx++;
    std::stringstream AssStream;
    AssStream << std::setw(8) << std::setfill('0') << ii.assemblyLine;
    out << AssStream.str();
    out << " in " << f->getName().str() << " (";
    // Yawn, we could go up and print varargs if we wanted to.
    unsigned index = 0;
    for (Function::arg_iterator ai = f->arg_begin(), ae = f->arg_end();
         ai != ae; ++ai) {
      if (ai!=f->arg_begin()) out << ", ";

      out << ai->getName().str();
      // XXX should go through function
      ref<Expr> value = sf.locals[sf.kf->getArgRegister(index++)].value;
      if (isa_and_nonnull<ConstantExpr>(value))
        out << "=" << value;
    }
    out << ")";
    if (ii.file != "")
      out << " at " << ii.file << ":" << ii.line;
    out << "\n";
    target = sf.caller;
  }
}

void ExecutionState::addConstraint(ref<Expr> e) {
  ConstraintManager c(constraints);
  c.addConstraint(e);
}

Thread* ExecutionState::findThreadById(unsigned threadId) {
	return threadList.findThreadById(threadId);
}

Thread* ExecutionState::getCurrentThread() {
	if (!threadScheduler->isSchedulerEmpty()) {
		currentThread = threadScheduler->selectCurrentItem();
	} else {
		currentThread = NULL;
	}
	return currentThread;
}

Thread* ExecutionState::getNextThread() {
	if (threadScheduler->isSchedulerEmpty()) {
    assert(0 && "No thread scheduller is ready.");
	}
	currentThread = threadScheduler->selectNextItem();
	currentStack = currentThread->stack;
	return currentThread;
}

bool ExecutionState::examineAllThreadFinalState() {
  bool isAllThreadFinished = true;
  for (ThreadList::iterator ti = threadList.begin(), te = threadList.end();
       ti != te; ti++) {
    Thread *thread = *ti;
    unsigned line;
    std::string file, dir;
    if (!thread->isTerminated()) {
      isAllThreadFinished = false;
      Instruction *inst = thread->prevPC->inst;
      std::cerr << "thread " << thread->threadId
                << " unable to finish successfully, final state is "
                << thread->threadState << std::endl;
      std::cerr << "function = "
                << inst->getParent()->getParent()->getName().str() << std::endl;
      if (const auto &DL = inst->getDebugLoc()) {
        line = DL->getLine();
        file = DL->getFilename().str();
        dir = DL->getDirectory().str();
        std::cerr << "pos = " << dir << "/" << file << " : " << line << " "
                  << inst->getOpcodeName() << std::endl;
      }
      std::cerr << std::endl;
    }
  }
  threadScheduler->printAllItem(std::cerr);
  return isAllThreadFinished;
}

unsigned ExecutionState::getNextThreadId() {
	unsigned threadId = nextThreadId++;
	assert (nextThreadId < 17 && "vector clock 只有16个");
	return threadId;
}

Thread* ExecutionState::createThread(KFunction *kf) {
	Thread* newThread = new Thread(getNextThreadId(), currentThread, kf, &addressSpace);
	threadList.addThread(newThread);
	threadScheduler->addItem(newThread);
	return newThread;
}

Thread* ExecutionState::createThread(KFunction *kf, unsigned threadId) {
	if (threadId >= nextThreadId) {
		nextThreadId = threadId + 1;
		assert (nextThreadId < 17 && "vector clock 只有16个");
	}
	Thread* newThread = new Thread(threadId, currentThread, kf, &addressSpace);
	threadList.addThread(newThread);
	threadScheduler->addItem(newThread);
	return newThread;

}

void ExecutionState::swapOutThread(Thread* thread, bool isCondBlocked, bool isBarrierBlocked, bool isJoinBlocked, bool isTerminated) {
	threadScheduler->removeItem(thread);
	if (isCondBlocked) {
		thread->threadState = Thread::COND_BLOCKED;
	}
	if (isBarrierBlocked) {
		thread->threadState = Thread::BARRIER_BLOCKED;
	}
	if (isJoinBlocked) {
		thread->threadState = Thread::JOIN_BLOCKED;
	}
	if (isTerminated) {
		thread->threadState = Thread::TERMINATED;
	}
}

void ExecutionState::swapInThread(Thread* thread, bool isRunnable, bool isMutexBlocked) {
	threadScheduler->addItem(thread);
	if (isRunnable) {
		thread->threadState = Thread::RUNNABLE;
	}
	if (isMutexBlocked) {
		thread->threadState = Thread::MUTEX_BLOCKED;
	}
}

void ExecutionState::switchThreadToMutexBlocked(Thread* thread) {
	assert(thread->isRunnable());
	thread->threadState = Thread::MUTEX_BLOCKED;
}

void ExecutionState::switchThreadToRunnable(Thread* thread) {
	assert(thread->isMutexBlocked());
	thread->threadState = Thread::RUNNABLE;
}

void ExecutionState::swapOutThread(unsigned threadId, bool isCondBlocked, bool isBarrierBlocked, bool isJoinBlocked, bool isTerminated) {
	swapOutThread(findThreadById(threadId), isCondBlocked, isBarrierBlocked, isJoinBlocked, isTerminated);
}

void ExecutionState::swapInThread(unsigned threadId, bool isRunnable, bool isMutexBlocked) {
	swapInThread(findThreadById(threadId), isRunnable, isMutexBlocked);
}

void ExecutionState::switchThreadToMutexBlocked(unsigned threadId) {
	switchThreadToMutexBlocked(findThreadById(threadId));
}

void ExecutionState::switchThreadToRunnable(unsigned threadId) {
	switchThreadToRunnable(findThreadById(threadId));
}

void ExecutionState::reSchedule() {
	threadScheduler->reSchedule();
}
