//===-- CondScheduler.h -----------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LIB_CORE_CONDSCHEDULER_H_
#define LIB_CORE_CONDSCHEDULER_H_

#include "klee/Encode/Prefix.h"
#include "klee/Thread/WaitParam.h"

#include <iostream>
#include <list>
#include <vector>

namespace klee {

class CondScheduler {
public:
  enum CondSchedulerType
  {
    FIFS,
    Preemptive,
    Random
  };
  CondScheduler();
  virtual ~CondScheduler();
  virtual WaitParam *selectNextItem() = 0;
  virtual void popAllItem(std::vector<WaitParam *> &allItem) = 0;
  virtual int itemNum() = 0;
  virtual bool isQueueEmpty() = 0;
  virtual void printName(std::ostream &os) = 0;
  virtual void addItem(WaitParam *param) = 0;
  virtual WaitParam *removeItem(WaitParam *param) = 0;
  virtual WaitParam *removeItem(unsigned threadId) = 0;
  virtual void printAllItem(std::ostream &os) = 0;
};

class FIFSCondScheduler : public CondScheduler {
private:
  std::list<WaitParam *> queue;

public:
  void printName(std::ostream &os) {
    os << "FIFS Condition Scheduler\n";
  }
  FIFSCondScheduler();
  virtual ~FIFSCondScheduler();
  WaitParam *selectNextItem();
  void popAllItem(std::vector<WaitParam *> &allItem);
  int itemNum();
  bool isQueueEmpty();
  void addItem(WaitParam *param);
  WaitParam *removeItem(WaitParam *param);
  WaitParam *removeItem(unsigned threadId);
  void printAllItem(std::ostream &os);
};

class PreemptiveCondScheduler : public CondScheduler {
private:
  std::list<WaitParam *> queue;

public:
  void printName(std::ostream &os) {
    os << "Preemptive Condition Scheduler\n";
  }
  PreemptiveCondScheduler();
  virtual ~PreemptiveCondScheduler();
  WaitParam *selectNextItem();
  void popAllItem(std::vector<WaitParam *> &allItem);
  int itemNum();
  bool isQueueEmpty();
  void addItem(WaitParam *param);
  WaitParam *removeItem(WaitParam *param);
  WaitParam *removeItem(unsigned threadId);
  void printAllItem(std::ostream &os);
};

class GuidedCondScheduler : public CondScheduler {
private:
  CondScheduler *baseScheduler;
  Prefix *prefix;

public:
  void printName(std::ostream &os) {
    os << "Guided Condition Scheduler\n";
  }
  GuidedCondScheduler(CondSchedulerType secondarySchedulerType, Prefix *prefix);
  virtual ~GuidedCondScheduler();
  WaitParam *selectNextItem();
  void popAllItem(std::vector<WaitParam *> &allItem);
  int itemNum();
  bool isQueueEmpty();
  void addItem(WaitParam *param);
  WaitParam *removeItem(WaitParam *param);
  WaitParam *removeItem(unsigned threadId);
  void printAllItem(std::ostream &os);
};

CondScheduler *getCondSchedulerByType(CondScheduler::CondSchedulerType type);

} /* namespace klee */

#endif /* LIB_CORE_CONDSCHEDULER_H_ */
