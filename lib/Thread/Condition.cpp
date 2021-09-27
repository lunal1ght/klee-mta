//===-- Condition.cpp -------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Condition.h"

#include <iterator>

using namespace ::std;

namespace klee {

Condition::Condition() {}

Condition::Condition(unsigned id, string name, CondScheduler::CondSchedulerType scheduleType) : id(id), name(name) {
  waitingList = getCondSchedulerByType(scheduleType);
}

Condition::Condition(unsigned id, string name, CondScheduler::CondSchedulerType schedulerType, Prefix *prefix)
    : id(id), name(name) {
  waitingList = new GuidedCondScheduler(schedulerType, prefix);
}

void Condition::wait(WaitParam *waitParam) { waitingList->addItem(waitParam); }

WaitParam *Condition::signal() {
  if (waitingList->isQueueEmpty()) {
    return NULL;
  } else {
    WaitParam *wp = waitingList->selectNextItem();
    return wp;
  }
}

void Condition::broadcast(vector<WaitParam *> &allWait) { 
  waitingList->popAllItem(allWait); 
}

Condition::~Condition() {
  // release unsignaled thread
  if (!waitingList->isQueueEmpty()) {
    vector<WaitParam *> allItem;
    waitingList->popAllItem(allItem);
    for (vector<WaitParam *>::iterator wi = allItem.begin(), we = allItem.end(); wi != we; wi++) {
      delete *wi;
    }
  }
  delete this->waitingList;
}

} // namespace klee
