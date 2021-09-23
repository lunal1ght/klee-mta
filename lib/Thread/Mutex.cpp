//===-- Mutex.cpp ----------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "klee/Thread/Mutex.h"

using namespace ::std;

namespace klee {

Mutex::Mutex(unsigned id, string name) : id(id), name(name), isLocked(false), lockedThreadId(0) {}

Mutex::~Mutex() {}

void Mutex::lock(unsigned threadId) {
  this->lockedThreadId = threadId;
  this->isLocked = true;
}

void Mutex::unlock() {
  this->lockedThreadId = 0;
  this->isLocked = false;
}

bool Mutex::isThreadOwnMutex(unsigned threadId) {
  if (threadId == lockedThreadId) {
    return true;
  } else {
    return false;
  }
}
// void Mutex::addToBlockedList(Thread* thread) {
//	blockedList.push_front(thread);
//}
//
// void Mutex::removeFromBlockedList(Thread* thread) {
//	blockedList.remove(thread);
//}

} // namespace klee
