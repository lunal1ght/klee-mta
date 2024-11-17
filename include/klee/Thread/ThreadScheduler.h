//===-- ThreadScheduler.h ---------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef LIB_CORE_THREADSCHEDULER_H_
#define LIB_CORE_THREADSCHEDULER_H_

#include <iostream>
#include <list>
#include <map>
#include <vector>
#include <memory>

#include "klee/Thread/Thread.h"

namespace klee {
class Prefix;
class ExecutionState;

class ThreadScheduler {
public:
  enum ThreadSchedulerType
  {
    RR,
    FIFS,
    Preemptive
  };
  ThreadScheduler() = default;
  virtual ~ThreadScheduler() = default;
  virtual Thread* selectCurrentItem() = 0;
  virtual Thread *selectNextItem() = 0;
  virtual void popAllItem(std::vector<Thread*>& allItem) = 0; //  Исправлено
  virtual int itemNum() = 0;
  virtual bool isSchedulerEmpty() = 0;
  virtual void printName(std::ostream &os) = 0;
  virtual void addItem(Thread *item) = 0;
  virtual void removeItem(Thread *item) = 0;
  virtual void printAllItem(std::ostream &os) = 0;
  virtual void reSchedule() = 0;
  virtual ThreadScheduler::ThreadSchedulerType getType() const = 0;        // Виртуальный getType()
  virtual const std::list<Thread*>& getQueue() const = 0; // Виртуальный getQueue()
  virtual ThreadScheduler* clone() const = 0;           // Виртуальный clone() 
  virtual void setQueue(std::list<Thread*> queue) = 0;
};

/**
 * RR Scheduler
 */
class RRThreadScheduler : public ThreadScheduler {
private:
  std::list<Thread *> queue;
  unsigned int count;

public:
  RRThreadScheduler();
  RRThreadScheduler(const RRThreadScheduler& scheduler);
  RRThreadScheduler(RRThreadScheduler &scheduler, std::map<unsigned, Thread *> &threadMap);
  ~RRThreadScheduler() override = default;  // Деструктор больше не нужен
  void printName(std::ostream &os) {
    os << "RR Thread Scheduler\n";
  }

  Thread *selectCurrentItem();
  Thread *selectNextItem();
  void popAllItem(std::vector<Thread *> &allItem);
  int itemNum();
  bool isSchedulerEmpty();
  void addItem(Thread *item);
  void removeItem(Thread *item);
  void printAllItem(std::ostream &os);
  void reSchedule();
  void setCountZero();
  ThreadSchedulerType getType() const override;
  const std::list<Thread*>& getQueue() const override;
  ThreadScheduler* clone() const override; // Объявление clone()
  void setQueue(std::list<Thread*> queue) override;
};

/**
 * FIFS Scheduler, simply queues threads in the order that they arrive in the ready queue
 */
class FIFSThreadScheduler : public ThreadScheduler {
private:
  std::list<Thread *> queue;

public:
  FIFSThreadScheduler();
  FIFSThreadScheduler(const FIFSThreadScheduler& scheduler);
  FIFSThreadScheduler(FIFSThreadScheduler &scheduler, std::map<unsigned, Thread *> &threadMap);
  ~FIFSThreadScheduler() override = default;
  void printName(std::ostream &os) {
    os << "FIFS Thread Scheduler\n";
  }

  Thread *selectCurrentItem();
  Thread *selectNextItem();
  void popAllItem(std::vector<Thread *> &allItem);
  int itemNum();
  bool isSchedulerEmpty();
  void addItem(Thread *item);
  void removeItem(Thread *item);
  void printAllItem(std::ostream &os);
  void reSchedule();
  ThreadSchedulerType getType() const override;
  const std::list<Thread*>& getQueue() const override;
  ThreadScheduler* clone() const override; // Объявление clone()
  void setQueue(std::list<Thread*> queue) override;
};

class PreemptiveThreadScheduler : public ThreadScheduler {
private:
  std::list<Thread *> queue;

public:
  PreemptiveThreadScheduler();
  PreemptiveThreadScheduler(const PreemptiveThreadScheduler& scheduler);
  PreemptiveThreadScheduler(PreemptiveThreadScheduler &scheduler, std::map<unsigned, Thread *> &threadMap);
  ~PreemptiveThreadScheduler() override = default;
  void printName(std::ostream &os) {
    os << "Preemptive Thread Scheduler\n";
  }

  Thread *selectCurrentItem();
  Thread *selectNextItem();
  void popAllItem(std::vector<Thread *> &allItem);
  int itemNum();
  bool isSchedulerEmpty();
  void addItem(Thread *item);
  void removeItem(Thread *item);
  void printAllItem(std::ostream &os);
  void reSchedule();
  ThreadSchedulerType getType() const override;
  const std::list<Thread*>& getQueue() const override;
  ThreadScheduler* clone() const override; // Объявление clone()
  void setQueue(std::list<Thread*> queue) override;
};

class GuidedThreadScheduler : public ThreadScheduler {
private:
  Prefix *prefix;
  ThreadScheduler *subScheduler;
  ExecutionState *state;

public:
  GuidedThreadScheduler(ExecutionState *state, ThreadSchedulerType schedulerType, Prefix *prefix);
  GuidedThreadScheduler(const GuidedThreadScheduler& other);  
  ~GuidedThreadScheduler() override = default;
  void printName(std::ostream &os) {
    os << "Guided Thread Scheduler\n";
  }

  Thread *selectCurrentItem();
  Thread *selectNextItem();
  void popAllItem(std::vector<Thread *> &allItem);
  int itemNum();
  bool isSchedulerEmpty();
  void addItem(Thread *item);
  void removeItem(Thread *item);
  void printAllItem(std::ostream &os);
  void reSchedule();
  ThreadSchedulerType getType() const override;
  const std::list<Thread*>& getQueue() const override;
  ThreadScheduler* clone() const override;
  void setQueue(std::list<Thread*> queue) override;
};

ThreadScheduler *getThreadSchedulerByType(ThreadScheduler::ThreadSchedulerType type);

} /* namespace klee */

#endif /* LIB_CORE_THREADSCHEDULER_H_ */
