//===-- Thread.cpp ----------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "klee/Thread/Thread.h"

namespace klee {

Thread::Thread(unsigned threadId, Thread *parentThread, KFunction *kf, AddressSpace *addressSpace)
    : pc(kf->instructions), prevPC(pc), incomingBBIndex(0), threadId(threadId), parentThread(parentThread),
      threadState(Thread::RUNNABLE), addressSpace(addressSpace) {
  for (unsigned i = 0; i < 17; i++) {
    vectorClock.push_back(0);
  }
  stack = new StackType(addressSpace);
  stack->realStack.reserve(10);
  stack->pushFrame(0, kf);
}

Thread::Thread(Thread &anotherThread, AddressSpace *addressSpace)
    : pc(anotherThread.pc), prevPC(anotherThread.prevPC), incomingBBIndex(anotherThread.incomingBBIndex),
      threadId(anotherThread.threadId), parentThread(anotherThread.parentThread),
      threadState(anotherThread.threadState), addressSpace(addressSpace) {
  stack = new StackType(addressSpace, anotherThread.stack);
  for (unsigned i = 0; i < 17; i++) {
    vectorClock.push_back(0);
  }
}

Thread::Thread(const Thread& other) 
    : pc(other.pc),
      prevPC(other.prevPC),
      incomingBBIndex(other.incomingBBIndex),
      threadId(other.threadId),
      parentThread(other.parentThread), //  Проверьте, нужно ли копировать или просто присвоить указатель
      threadState(other.threadState),
      addressSpace(other.addressSpace), // Проверьте, нужно ли копировать или просто присвоить указатель
      vectorClock(other.vectorClock) // Копируем vectorClock
{
    stack = new StackType(*other.stack); // Глубокое копирование StackType
}

Thread::~Thread() {
  delete stack;
}

void Thread::dumpStack(llvm::raw_ostream &out) const {
  stack->dumpStack(out, prevPC);
}

} /* namespace klee */
