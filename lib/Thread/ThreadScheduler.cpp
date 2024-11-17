//===-- ThreadScheduler.cpp -------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <cassert>
#include <string>

#include "../Core/ExecutionState.h"
#include "klee/Module/InstructionInfoTable.h"
#include "klee/Module/KInstruction.h"
#include "klee/Thread/ThreadScheduler.h"
#include <llvm/IR/Instruction.h>
#include <queue>

#define MAXINST 100

using namespace ::std;

namespace klee {

ThreadScheduler* getThreadSchedulerByType(ThreadScheduler::ThreadSchedulerType type) {
    //  Вместо  new  используйте  clone()
    switch (type) {
    case ThreadScheduler::RR: {
        static RRThreadScheduler prototype;  // Статический экземпляр-прототип
        return prototype.clone();
    }
    case ThreadScheduler::FIFS: {
        static FIFSThreadScheduler prototype;
        return prototype.clone();
    }
    case ThreadScheduler::Preemptive: {
        static PreemptiveThreadScheduler prototype;
        return prototype.clone();
    }
    default: {
        assert("ThreadSchedulerType error");
        return nullptr;  // Или бросьте исключение
    }
  }   
}
//ThreadScheduler::ThreadScheduler() {}

//ThreadScheduler::~ThreadScheduler() {}

RRThreadScheduler::RRThreadScheduler() {
  count = 0;
}

// 拷贝构造，没用
RRThreadScheduler::RRThreadScheduler(const RRThreadScheduler &scheduler) : count(scheduler.count) {
  for (Thread *thread : scheduler.queue) {
    queue.push_back(new Thread(*thread)); // Глубокое копирование
  }
}

/*RRThreadScheduler::~RRThreadScheduler() {
  for (Thread* thread : queue) {
    delete thread;
  }
}*/

ThreadScheduler::ThreadSchedulerType RRThreadScheduler::getType() const { return ThreadScheduler::RR; }
const std::list<Thread*>& RRThreadScheduler::getQueue() const { return queue; }
ThreadScheduler* RRThreadScheduler::clone() const { return new RRThreadScheduler(*this); }

void RRThreadScheduler::setQueue(std::list<Thread*> newQueue) {
  queue = std::move(newQueue); // используем std::move для эффективности
}

// 考虑使用迭代器而不是front();
Thread *RRThreadScheduler::selectCurrentItem() {
  return queue.front();
}

Thread *RRThreadScheduler::selectNextItem() {
  if (count > MAXINST) {
    reSchedule();
  }
  count++;
  return queue.front();
}

void RRThreadScheduler::popAllItem(vector<Thread *> &allItem) {
  allItem.reserve(queue.size());
  for (list<Thread *>::iterator ti = queue.begin(), te = queue.end(); ti != te; ti++) {
    allItem.push_back(*ti);
  }
  queue.clear();
}

int RRThreadScheduler::itemNum() {
  return queue.size();
}

bool RRThreadScheduler::isSchedulerEmpty() {
  return queue.empty();
}

void RRThreadScheduler::addItem(Thread *item) {
  queue.push_back(item);
}

void RRThreadScheduler::removeItem(Thread *item) {
  for (list<Thread *>::iterator ti = queue.begin(), te = queue.end(); ti != te; ti++) {
    if (*ti == item) {
      queue.erase(ti);
      break;
    }
  }
}

void RRThreadScheduler::printAllItem(ostream &os) {
  for (list<Thread *>::iterator ti = queue.begin(), te = queue.end(); ti != te; ti++) {
    Thread *thread = *ti;
    os << thread->threadId << " state: " << thread->threadState
       << " current inst: " << thread->pc->inst->getOpcodeName() << " ";
    if (thread->threadState == Thread::TERMINATED) {
      KInstruction *ki = thread->pc;
      os << ki->info->file << " " << ki->info->line;
    }
    os << endl;
  }
}

void RRThreadScheduler::reSchedule() {
  Thread *thread = queue.front();
  queue.pop_front();
  queue.push_back(thread);
  count = 0;
}

void RRThreadScheduler::setCountZero() {
  count = 0;
}

FIFSThreadScheduler::FIFSThreadScheduler() {}

FIFSThreadScheduler::FIFSThreadScheduler(const FIFSThreadScheduler& scheduler) {  // <<< ДОБАВЛЕНО
  for (Thread* thread : scheduler.queue) {
    queue.push_back(new Thread(*thread)); // Глубокое копирование
  }
}

/*FIFSThreadScheduler::~FIFSThreadScheduler() {
  for (Thread* thread : queue) {
    delete thread;
  }
}*/

ThreadScheduler::ThreadSchedulerType FIFSThreadScheduler::getType() const { return ThreadScheduler::FIFS; }
const std::list<Thread*>& FIFSThreadScheduler::getQueue() const { return queue; }
ThreadScheduler* FIFSThreadScheduler::clone() const { return new FIFSThreadScheduler(*this); }

void FIFSThreadScheduler::setQueue(std::list<Thread*> newQueue) {
  queue = std::move(newQueue); 
}

Thread *FIFSThreadScheduler::selectCurrentItem() {
  return selectNextItem();
}

Thread *FIFSThreadScheduler::selectNextItem() {
  return queue.front();
}

void FIFSThreadScheduler::popAllItem(vector<Thread *> &allItem) {
  allItem.reserve(queue.size());
  for (list<Thread *>::iterator ti = queue.begin(), te = queue.end(); ti != te; ti++) {
    allItem.push_back(*ti);
  }
  queue.clear();
}

int FIFSThreadScheduler::itemNum() {
  return queue.size();
}

bool FIFSThreadScheduler::isSchedulerEmpty() {
  return queue.empty();
}

void FIFSThreadScheduler::addItem(Thread *item) {
  queue.push_back(item);
}

void FIFSThreadScheduler::removeItem(Thread *item) {
  for (list<Thread *>::iterator ti = queue.begin(), te = queue.end(); ti != te; ti++) {
    if (*ti == item) {
      queue.erase(ti);
      break;
    }
  }
}

void FIFSThreadScheduler::printAllItem(ostream &os) {
  for (list<Thread *>::iterator ti = queue.begin(), te = queue.end(); ti != te; ti++) {
    Thread *thread = *ti;
    os << thread->threadId << " state: " << thread->threadState
       << " current inst: " << thread->pc->inst->getOpcodeName() << " ";
    if (thread->threadState == Thread::TERMINATED) {
      KInstruction *ki = thread->pc;
      os << ki->info->file << " " << ki->info->line;
    }
    os << endl;
  }
}

void FIFSThreadScheduler::reSchedule() {
  Thread *thread = queue.front();
  queue.pop_front();
  queue.push_back(thread);
}

PreemptiveThreadScheduler::PreemptiveThreadScheduler() {}

PreemptiveThreadScheduler::PreemptiveThreadScheduler(const PreemptiveThreadScheduler& scheduler) { // <<< ДОБАВЛЕНО
  for (Thread* thread : scheduler.queue) {
    queue.push_back(new Thread(*thread)); // Глубокое копирование
  }
}

/*PreemptiveThreadScheduler::~PreemptiveThreadScheduler() {
  for (Thread* thread : queue) {
    delete thread;
  }
}*/

ThreadScheduler::ThreadSchedulerType PreemptiveThreadScheduler::getType() const { return ThreadScheduler::Preemptive; }
const std::list<Thread*>& PreemptiveThreadScheduler::getQueue() const { return queue; }
ThreadScheduler* PreemptiveThreadScheduler::clone() const { return new PreemptiveThreadScheduler(*this); }

void PreemptiveThreadScheduler::setQueue(std::list<Thread*> newQueue) {
  queue = std::move(newQueue);
}

Thread *PreemptiveThreadScheduler::selectCurrentItem() {
  return selectNextItem();
}

Thread *PreemptiveThreadScheduler::selectNextItem() {
  return queue.back();
}

void PreemptiveThreadScheduler::popAllItem(std::vector<Thread *> &allItem) {
  allItem.reserve(queue.size());
  for (list<Thread *>::iterator ti = queue.begin(), te = queue.end(); ti != te; ti++) {
    allItem.push_back(*ti);
  }
  queue.clear();
}

int PreemptiveThreadScheduler::itemNum() {
  return queue.size();
}

bool PreemptiveThreadScheduler::isSchedulerEmpty() {
  return queue.empty();
}

void PreemptiveThreadScheduler::addItem(Thread *item) {
  queue.push_back(item);
}

void PreemptiveThreadScheduler::removeItem(Thread *item) {
  for (list<Thread *>::iterator ti = queue.begin(), te = queue.end(); ti != te; ti++) {
    if (*ti == item) {
      queue.erase(ti);
      break;
    }
  }
}

void PreemptiveThreadScheduler::printAllItem(std::ostream &os) {
  for (list<Thread *>::iterator ti = queue.begin(), te = queue.end(); ti != te; ti++) {
    Thread *thread = *ti;
    os << thread->threadId << " state: " << thread->threadState
       << " current inst: " << thread->pc->inst->getOpcodeName() << " ";
    if (thread->threadState == Thread::TERMINATED) {
      KInstruction *ki = thread->pc;
      os << ki->info->file << " " << ki->info->line;
    }
    os << endl;
  }
}

void PreemptiveThreadScheduler::reSchedule() {
  Thread *thread = queue.back();
  queue.pop_back();
  list<Thread *>::iterator ti = queue.end();
  ti--;
  queue.insert(ti, thread);
}

GuidedThreadScheduler::GuidedThreadScheduler(ExecutionState *state, ThreadSchedulerType schedulerType, Prefix *prefix)
    : prefix(prefix), state(state) {
  subScheduler = getThreadSchedulerByType(schedulerType);
}

GuidedThreadScheduler::GuidedThreadScheduler(const GuidedThreadScheduler &other) 
    : prefix(other.prefix), state(other.state) {
  subScheduler = other.subScheduler->clone();
}

ThreadScheduler::ThreadSchedulerType GuidedThreadScheduler::getType() const { //  Полное имя типа
  assert(subScheduler && "subScheduler is null");
  return subScheduler->getType();
}

const std::list<Thread*>& GuidedThreadScheduler::getQueue() const {
  assert(subScheduler && "subScheduler is null");
  return subScheduler->getQueue();
}

ThreadScheduler* GuidedThreadScheduler::clone() const {
    return new GuidedThreadScheduler(*this);
}


/*GuidedThreadScheduler::~GuidedThreadScheduler() {
    delete subScheduler;
    subScheduler = nullptr;
}*/

void GuidedThreadScheduler::setQueue(std::list<Thread*> newQueue) {

    std::list<Thread*> copiedQueue;
    for (Thread* thread : newQueue) {
        copiedQueue.push_back(new Thread(*thread));  // Создаем новые копии потоков
    }
    subScheduler->setQueue(std::move(copiedQueue)); //  Обновляем очередь подпланировщика
}

Thread *GuidedThreadScheduler::selectCurrentItem() {
  return selectNextItem();
}

Thread *GuidedThreadScheduler::selectNextItem() {
  Thread *thread = NULL;
  if (!prefix->isFinished()) {
    unsigned threadId = prefix->getCurrentEventThreadId();
    thread = state->findThreadById(threadId);
  } else {
    thread = subScheduler->selectNextItem();
  }
  return thread;
}

void GuidedThreadScheduler::popAllItem(vector<Thread *> &allItem) {
  subScheduler->popAllItem(allItem);
}

int GuidedThreadScheduler::itemNum() {
  return subScheduler->itemNum();
}

bool GuidedThreadScheduler::isSchedulerEmpty() {
  return subScheduler->isSchedulerEmpty();
}

void GuidedThreadScheduler::addItem(Thread *item) {
  subScheduler->addItem(item);
}

void GuidedThreadScheduler::removeItem(Thread *item) {
  subScheduler->removeItem(item);
}

void GuidedThreadScheduler::printAllItem(std::ostream &os) {
  subScheduler->printAllItem(os);
}

void GuidedThreadScheduler::reSchedule() {
  subScheduler->reSchedule();
}

} /* namespace klee */
