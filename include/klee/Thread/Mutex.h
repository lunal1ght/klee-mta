//===-- Mutex.h -------------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MUTEX_H_
#define MUTEX_H_

#include "klee/Thread/MutexScheduler.h"
#include <string>
#include "llvm/Support/raw_ostream.h"

namespace klee {

class Mutex {

public:
  unsigned id;
  std::string name;

private:
  bool isLocked;
  unsigned lockedThreadId;
  // std::list<Thread*> blockedList;
  // unsigned lockedThread;
  // MutexScheduler* blockedList;

public:

  Mutex(unsigned id, const std::string& name); // Обычный конструктор (изменил string на const string&)
  //Mutex(const Mutex& other);                   // Конструктор копирования

  bool isMutexLocked() {
    return isLocked;
  }

  bool isCurrentlyLocked() const;

  unsigned getLockedThread() {
    return lockedThreadId;
  }

  void lock(unsigned thread);
  void unlock();

  bool isThreadOwnMutex(unsigned threadId);
  //	void addToBlockedList(Thread* thread);
  //	void removeFromBlockedList(Thread* thread);

  virtual ~Mutex();
};

} // namespace klee
#endif /* MUTEX_H_ */
