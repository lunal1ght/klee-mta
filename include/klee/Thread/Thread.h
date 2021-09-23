//===-- Thread.h ------------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LIB_CORE_THREAD_H_
#define LIB_CORE_THREAD_H_

#include <vector>

#include "klee/Module/KInstIterator.h"
#include "klee/Thread/StackType.h"

namespace klee {

class Thread {
public:
  enum ThreadState
  {
    RUNNABLE,
    MUTEX_BLOCKED,
    COND_BLOCKED,
    BARRIER_BLOCKED,
    JOIN_BLOCKED,
    TERMINATED
  };

  KInstIterator pc, prevPC;
  unsigned incomingBBIndex;
  unsigned threadId;
  Thread *parentThread;
  ThreadState threadState;
  AddressSpace *addressSpace;
  StackType *stack;
  std::vector<unsigned> vectorClock;

public:
  Thread(unsigned threadId, Thread *parentThread, KFunction *kf, AddressSpace *addressSpace);
  Thread(Thread &anotherThread, AddressSpace *addressSpace);
  virtual ~Thread();

  bool isRunnable() {
    return threadState == RUNNABLE;
  }
  bool isMutexBlocked() {
    return threadState == MUTEX_BLOCKED;
  }
  bool isCondBlocked() {
    return threadState == COND_BLOCKED;
  }
  bool isBarrierBlocked() {
    return threadState == BARRIER_BLOCKED;
  }
  bool isJoinBlocked() {
    return threadState == JOIN_BLOCKED;
  }
  bool isTerminated() {
    return threadState == TERMINATED;
  }
  bool isSchedulable() {
    return isRunnable() || isMutexBlocked();
  }
  void dumpStack(llvm::raw_ostream &out) const;
};

} /* namespace klee */

#endif /* LIB_CORE_THREAD_H_ */
