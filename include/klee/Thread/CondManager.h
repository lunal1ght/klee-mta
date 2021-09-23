//===-- CondManager.h -------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef CONDMANAGER_H_
#define CONDMANAGER_H_

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "klee/Thread/Condition.h"

namespace klee {
class MutexManager;
} /* namespace klee */

namespace klee {

class CondManager {
private:
  std::map<std::string, Condition *> condPool;
  MutexManager *mutexManager;
  unsigned nextConditionId;

public:
  CondManager();
  CondManager(MutexManager *mutexManaget);
  virtual ~CondManager();
  bool wait(std::string condName, std::string mutexName, unsigned threadId, std::string &errorMsg);
  bool signal(std::string condName, unsigned &releasedThreadId, std::string &errorMsg);
  bool broadcast(std::string condName, std::vector<unsigned> &threads, std::string &errorMsg);
  bool addCondition(std::string condName, std::string &errorMsg);
  bool addCondition(std::string condName, std::string &errorMsg, Prefix *prefix);
  Condition *getCondition(std::string condName);
  void setMutexManager(MutexManager *mutexManager) {
    this->mutexManager = mutexManager;
  }
  void clear();
  void print(std::ostream &out);
  unsigned getNextConditionId();
};

} // namespace klee
#endif /* CONDMANAGER_H_ */
