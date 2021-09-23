//===-- Condition.h ---------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef CONDITION_H_
#define CONDITION_H_

#include <string>
#include <vector>

#include "klee/Thread/CondScheduler.h"
#include "klee/Thread/WaitParam.h"

namespace klee {

class Condition {
private:
  CondScheduler *waitingList;
  // Mutex* associatedMutex;

public:
  unsigned id;
  std::string name;

public:
  Condition();

  Condition(unsigned id, std::string name, CondScheduler::CondSchedulerType schedulerType);

  Condition(unsigned id, std::string name, CondScheduler::CondSchedulerType schedulerType, Prefix *prefix);

  void wait(WaitParam *waitParam);

  WaitParam *signal();

  void broadcast(std::vector<WaitParam *> &allWait);

  virtual ~Condition();
};

} // namespace klee
#endif /* CONDITION_H_ */
